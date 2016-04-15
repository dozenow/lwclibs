#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "shared_malloc.h"
#include "shared_ht.h"


size_t djb2_hash(void *str, size_t len)
{
	unsigned long hash = 5381;
	int c;

	for(size_t i = 0; i < len; ++i) {
		c = ((char*)str)[i];
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash;
}

void * voidcpy(void *src, size_t len, struct sh_memory_pool *mp) {
	char * dest = sh_malloc(len, mp);
	if (!dest)
		return NULL;
	memcpy(dest, src, len);
	return dest;
}


int ht_cmp(void *a, size_t a_len, void *b, size_t b_len) {
	return strcmp(a,b);
}

void * identitycpy(void *src, size_t len, struct sh_memory_pool *mp) {
	return src;
}

void nonfree(void *src, struct sh_memory_pool *mp) {
	return;
}

int main() {


#define SBUF_SIZE (4096*10000)
	int *sbuf = mmap(NULL, SBUF_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 1;
	}

	struct sh_memory_pool *mp = init_sh_mempool(sbuf, SBUF_SIZE);

	sh_ht_t *ht = sh_ht_create(ht_cmp, djb2_hash, voidcpy, identitycpy, sh_free, nonfree, mp);
	if (!ht) {
		printf("Could not create HT\n");
		return 1;
	}

	FILE *fp = fopen("/usr/share/dict/web2a", "r");
	if (!fp) {
		perror("Could not open file");
		return 1;
	}

	char line[128];
	size_t lineno = 1;
	while(fgets(line, 128, fp) == line) {
		size_t len = strlen(line);
		line[len-1] = '\0';
		int upval;
		upval = sh_ht_update(ht, line, len, (void*)lineno, sizeof(lineno));
		if (upval == 1) {
			printf("Item %s already existed\n", line);
		} else if (upval == 0) {
			//okay
		} else {
			printf("Error inserting line %zu, got %d\n", lineno, upval);
			return 1;
		}
		lineno++;
	}

	fseek(fp, 0, 0);

	lineno = 1;
	while(fgets(line, 128, fp) == line) {
		size_t len = strlen(line);
		line[len-1] = '\0';
		sh_ht_iterator_t * it = sh_ht_get(ht, line, len);
		if (it) {
			char *key;
			size_t keylen, lineno, valsize;
			sh_ht_kv(it, (void**) &key, &keylen, (void*) &lineno, &valsize);
			//printf("%s => %zu\n", key, lineno);
			free_ht_iterator(it);
		} else {
			printf("%s not found in ht!\n", line);
			return 1;
		}
		lineno++;
	}

	printf("The hash table is of size %zu and lineno is %zu\n", sh_ht_size(ht), lineno);


	sh_ht_destroy(&ht);

	struct mallinfo mi = sh_mallinfo(mp);
	printf("total allocated space is %zu and total free space is %zu and arena is %zu and mmap is %zu\n", mi.uordblks, mi.fordblks, mi.arena, mi.hblkhd);
	

	return 0;
}
