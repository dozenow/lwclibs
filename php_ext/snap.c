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

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_snap.h"
#include "snapper.h"
#include "SAPI.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* If you declare any globals in php_snap.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(snap)
*/

/* True global resources - no need for thread safety here */
static int le_snap;
static int snap_fd;
static HashTable snap_hash;

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

	if (time(NULL) < expiration) {
		int rv = snap(SNAP_TARGET_NOJUMP, NULL, SNAP_SHARE_FD);
		snprintf(buf, sizeof(buf), "snap rv = %d\n", rv);
		write(snap_fd, buf, strlen(buf));
		php_printf("%s", buf);
		if (rv == SNAP_FAILED) {
			php_error(E_ERROR, "SNAP FAILED: %s\n", strerror(errno));
		} else if (rv >= 0) {
			zval val;
			char *uri = SG(request_info).request_uri;
			zend_string *str = zend_string_init(uri, strlen(uri)+1, 1);
			ZVAL_LONG(&val, rv);
			zend_hash_update(&snap_hash, str, &val);
			zend_string_free(str);
		}
	} else {
		snprintf(buf, sizeof(buf), "%d <= %d\n", time(NULL), expiration);
		write(snap_fd, buf, strlen(buf));
	}

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

static void snap_hash_dtor(zval *zval) {
	/* close val */
	char buf[128];
	close(Z_LVAL_P(zval));
	snprintf(buf, sizeof(buf), "destroying hash item, %d\n", Z_LVAL_P(zval));
	write(snap_fd, buf, strlen(buf));
	return;
};

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(snap)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	snap_fd = open("/tmp/phpsnap.txt", O_CREAT | O_TRUNC | O_WRONLY);
	zend_hash_init(&snap_hash, 50, NULL, snap_hash_dtor, 1);
	char buf[] = "opened snap_fd and initialized HT\n";
	write(snap_fd, buf, strlen(buf));
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
	close(snap_fd);
	zend_hash_destroy(&snap_hash);
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
	char *uri = SG(request_info).request_uri;

	if (!uri)
		return SUCCESS; /* PHP CLI, don't care */

	char buf[128];

	snprintf(buf, sizeof(buf), "requested %s\n", uri);
	write(snap_fd, buf, strlen(buf));

	zend_string *str = zend_string_init(uri, strlen(uri)+1, 1);	
	zval *rv;
	if ((rv = zend_hash_find(&snap_hash, str)) != NULL) {
		snprintf(buf, sizeof(buf), "present in HT, val is %d. Trying to snap\n", Z_LVAL_P(rv));
		write(snap_fd, buf, strlen(buf));
		int ret = snap(Z_LVAL_P(rv), NULL, SNAP_NOTHING);
		if (ret) {
			snprintf(buf, sizeof(buf), "arrived post snap: rv=%d, err=%s\n", ret, strerror(errno));
		}
		
	} else {
		snprintf(buf, sizeof(buf), "not present in HT\n");
	}
	zend_string_free(str);

	write(snap_fd, buf, strlen(buf));
	php_printf("%s %s\n", uri, buf);

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
