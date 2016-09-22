/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared_malloc.h"
#undef HAVE_MMAP

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_snap.h"
#include "lwc.h"
#include "shared_ht.h"
#include "SAPI.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

//#define perr(...) fprintf(stderr, __VA_ARGS__)
//#define perr(...) do { fprintf(stderr, "pid: %d ", getpid()); fprintf(stderr, __VA_ARGS__); } while(0)
#define perr(...)

extern int php_hash_environment(void);



/* If you declare any globals in php_snap.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(snap)
*/

/* True global resources - no need for thread safety here */
static int le_snap;
static int snap_fd;
static sh_ht_t *snap_hash;
static struct sh_memory_pool *snap_mp;
static void *snap_mempool_addr;


/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("snap.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_snap_globals, snap_globals)
    STD_PHP_INI_ENTRY("snap.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_snap_globals, snap_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_snap_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_snap_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "snap", arg);

	RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/

/* {{{ proto int snapcache(int expiration)
   Returns 1, 0, or -1 depending on snap created, snap jumped, or snap failed */
PHP_FUNCTION(snapcache)
{
	int argc = ZEND_NUM_ARGS();

	zend_long expiration;

	if (zend_parse_parameters(argc, "l", &expiration) == FAILURE) 
		return;

	char buf[128];
	int retval = 42;
	if (1 /* not expired */) {
		/* share the file table */
		struct lwc_resource_specifier specs[2];
		specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
		specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

		int rv = lwccreate(specs, 1, NULL, NULL, NULL, 0);
		//perr("snap rv = %d\n", rv);
		if (rv == LWC_SWITCHED) {
			SG(global_request_time) = 0; //force time to update
			php_hash_environment();
			php_startup_auto_globals();
			//perr("req time=%f qs=%s\n", SG(global_request_time), SG(request_info).query_string);
			retval = 0;
		} else if (rv == LWC_FAILED) {
			retval = -1;
			perr("SNAP FAILED: %s\n", strerror(errno));
		} else if (rv >= 0) {
			retval = 1;
			char *uri = SG(request_info).request_uri;
			int upval = sh_ht_update(snap_hash, uri, strlen(uri)+1, (void*)(size_t)rv, sizeof(rv));
			if (upval == 1) {
				perr("Item at %s already existed. \n", uri);
			} else if (upval == 0) {
				perr("Item at %s inserted.\n", uri);
			}

		}
	} else {
		perr("snap expired %ld <= %ld\n", time(NULL), expiration);
	}

	RETURN_LONG(retval);

}
/* }}} */


/* {{{ php_snap_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_snap_init_globals(zend_snap_globals *snap_globals)
{
	snap_globals->global_value = 0;
	snap_globals->global_string = NULL;
}
*/
/* }}} */

static size_t djb2_hash(void *str, size_t len)
{
	unsigned long hash = 5381;
	int c;

	for(size_t i = 0; i < len; ++i) {
		c = ((char*)str)[i];
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash;
}

static void * voidcpy(void *src, size_t len, struct sh_memory_pool *mp) {
	char * dest = sh_malloc(len, mp);
	if (!dest)
		return NULL;
	memcpy(dest, src, len);
	return dest;
}


static int ht_cmp(void *a, size_t a_len, void *b, size_t b_len) {
	return strcmp(a,b);
}

static void * identitycpy(void *src, size_t len, struct sh_memory_pool *mp) {
	return src;
}

static void sfd_free(void *src, struct sh_memory_pool *mp) {
	int fd = (int) src;
	//close(fd);
	return;
}

#define MEMPOOL_MMAP_SIZE (4096*100)

static int do_init() {

	perr("snap minit pid=%d\n", getpid());
	snap_mempool_addr = mmap(NULL, MEMPOOL_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (snap_mempool_addr == MAP_FAILED) {
		perr("Snap minit failed in mmap: %s\n", strerror(errno));
		php_error(E_ERROR, "Snap minit failed in mmap: %s\n", strerror(errno));
		return FAILURE;
	}

	snap_mp = init_sh_mempool(snap_mempool_addr, MEMPOOL_MMAP_SIZE);
	if (!snap_mp) {
		perr("Failed to init mempool\n");
		php_error(E_ERROR, "Failed to init mempool\n");
		return FAILURE;
	}

	snap_hash = sh_ht_create(ht_cmp, djb2_hash, voidcpy, identitycpy, sh_free, sfd_free, snap_mp);
	if (!snap_hash) {
		snap_mp = NULL;
		munmap(snap_mempool_addr, MEMPOOL_MMAP_SIZE);
		perr("Failed to init snap_hash\n");
		php_error(E_ERROR, "Failed to init snap_hash\n");
	}

	/* create initial snap for filetable sharing access */
	struct lwc_resource_specifier specs[1];
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;
	int rv = lwccreate(specs, 1, NULL, NULL, NULL, 0);
	if (rv < 0) {
		perr("can't init a snap\n");
		php_error(E_ERROR, "I can't init a snap\n");
		return FAILURE;
	}

	perr("initialized HT");

	return SUCCESS;

}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(snap)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(snap)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/

#if 0
	close(snap_fd);
	//unlink("/tmp/phpsnap.txt");
	sh_ht_destroy(&snap_hash);
	snap_mp = NULL;
	munmap(snap_mempool_addr, MEMPOOL_MMAP_SIZE);
#endif


	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(snap)
{
#if defined(COMPILE_DL_SNAP) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	if (!snap_mempool_addr) {
		if (do_init() == FAILURE) {
			perr("failed to init\n");
			return FAILURE;
		};
	}

	char *uri = SG(request_info).request_uri;

	if (!uri)
		return SUCCESS; /* PHP CLI, don't care */

	char buf[128];

	perr("requested %s\n", uri);

	sh_ht_iterator_t *it = sh_ht_get(snap_hash, uri, strlen(uri)+1);
	if (it) {
		int fd;
		size_t fdsize;
		sh_ht_value(it, (void*) &fd, &fdsize);
		perr("-->present in HT, val is %d. Trying to switch\n", fd);
		free_ht_iterator(it);
		int ret = lwcdiscardswitch(fd, NULL, 0);
		perr("arrived post snap: rv=%d, err=%s\n", ret, strerror(errno));
	} else {
		perr("req not present in HT\n");
	}

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(snap)
{
	char buf[] = "finished a request\n";
	write(snap_fd, buf, sizeof(buf) - 1);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(snap)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "snap support", "enabled");
	php_info_print_table_header(2, "snap version", "0.0002");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ snap_functions[]
 *
 * Every user visible function must have an entry in snap_functions[].
 */
const zend_function_entry snap_functions[] = {
	PHP_FE(confirm_snap_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(snapcache,	NULL)
	PHP_FE_END	/* Must be the last line in snap_functions[] */
};
/* }}} */

/* {{{ snap_module_entry
 */
zend_module_entry snap_module_entry = {
	STANDARD_MODULE_HEADER,
	"snap",
	snap_functions,
	PHP_MINIT(snap),
	PHP_MSHUTDOWN(snap),
	PHP_RINIT(snap),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(snap),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(snap),
	PHP_SNAP_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SNAP
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(snap)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
