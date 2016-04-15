#include "shared_ht.h"
#include "shared_malloc.h"
#include "shared_llist.h"
#include <stdlib.h>

struct sh_ht_entry {
	void *key;
	size_t keylen;
	void *value;
	size_t vallen;	
};

struct sh_ht_iterator {
	struct sh_llist_iterator * lit;
	struct sh_ht *ht;
	size_t curbuck;
};

struct sh_ht {
	struct sh_memory_pool *mp;
	size_t (*keyhashfunc)(void*,size_t);
	int (*keycmpfunc)(void *,size_t, void *, size_t);
	void *(*keycopyfunc)(void *,size_t, struct sh_memory_pool *);
	void *(*valuecopyfunc)(void*,size_t, struct sh_memory_pool *);
	void (*keyfreefunc)(void*, struct sh_memory_pool *);
	void (*valuefreefunc)(void*, struct sh_memory_pool *);
	sh_llist_t **buckets;
	size_t bucket_size;
	size_t size;
};


/* create takes a comparison function, a copy function, a free
 * function, and a memory pool. Any items passed into update will have
 * copy called on them. All items will have their freefuncs called on
 * them eventually. If the buckets are resized, they may be copied and
 * freed. Any copy functions can return NULL on error
 */
sh_ht_t *sh_ht_create(int (*cmpfunc)(void*, size_t, void*, size_t),
                      size_t (*keyhashfunc)(void*,size_t),
                      void *(*keycopyfunc)(void*,size_t, struct sh_memory_pool *),
                      void *(*valuecopyfunc)(void*,size_t, struct sh_memory_pool *), 
                      void (*keyfreefunc)(void*, struct sh_memory_pool *),
                      void (*valuefreefunc)(void*, struct sh_memory_pool *),
                      struct sh_memory_pool *mp) { 



	sh_ht_t *ht = sh_malloc(sizeof(*ht), mp);
	if (!ht)
		return NULL;
	ht->bucket_size = 797;
	ht->buckets = sh_calloc(ht->bucket_size, sizeof(ht->buckets[0]), mp);
	if (!ht->buckets) {
		sh_free(ht, mp);
		return NULL;
	}
	ht->size = 0;
	ht->keycmpfunc = cmpfunc;
	ht->keyhashfunc =  keyhashfunc;
	ht->keycopyfunc = keycopyfunc;
	ht->valuecopyfunc = valuecopyfunc;
	ht->keyfreefunc = keyfreefunc;
	ht->valuefreefunc = valuefreefunc;
	ht->mp = mp;
	return ht;
}

void free_ht_iterator(sh_ht_iterator_t *it) {
	sh_free(it, it->ht->mp);
}

/* just start looking from bucket from */
static sh_ht_iterator_t * sh_ht_from(sh_ht_t *ht, size_t from) {
	sh_ht_iterator_t *rv = NULL;
	for(size_t i = from; i < ht->bucket_size; ++i) {
		if (ht->buckets[i]) {
			rv = sh_malloc(sizeof(*rv), ht->mp);
			if (rv) {
				rv->lit = sh_llist_begin(ht->buckets[i]);
				rv->curbuck = i;
				rv->ht = ht;
			}
			break;
		}
	}
	return rv;	
}

/* return an iterator to the beginning of the ht. Returns item iterator */
sh_ht_iterator_t * sh_ht_begin(sh_ht_t *ht) {
	return sh_ht_from(ht, 0);
}

/* add/update an item to the ht. Returns 1 if the item already existed and 0 otherwise. -1 on error */
int sh_ht_update(sh_ht_t *ht, void *key, size_t keylen, void *value, size_t valuelen) {
	size_t idx = ht->keyhashfunc(key, keylen) % ht->bucket_size;
	int rv = 0;

	if (ht->buckets[idx]) {
		sh_llist_iterator_t *cur = sh_llist_begin(ht->buckets[idx]);
		for(;cur;cur=sh_llist_next(cur)) {
			struct sh_ht_entry *old = sh_llist_payload(cur);
			if (old->keylen == keylen &&
			    ht->keycmpfunc(old->key, old->keylen,
			                   key, keylen) == 0) {
				void *newd = ht->valuecopyfunc(value, valuelen, ht->mp);
				if (newd) {
					ht->valuefreefunc(old->value, ht->mp);
					old->value = newd;
					old->vallen = valuelen;
					rv = 1;
				} else {
					rv = -1;
				}
				goto out;
			}
		}
		// okay not in the bucket. Have to insert.
		
	} else {
		/* make the list to shove it into, that's all */
		ht->buckets[idx] = sh_llist_create(ht->mp);
		if (!ht->buckets[idx]) {
			rv = -1;
			goto out;
		}

	}

	/* still here? put it in the bucket */

	struct sh_ht_entry *ent = sh_malloc(sizeof(*ent), ht->mp);
	if (ent) {
		ent->key = ht->keycopyfunc(key, keylen, ht->mp);
		ent->keylen = keylen;
		ent->value = ht->valuecopyfunc(value, valuelen, ht->mp);
		ent->vallen = valuelen;
		if (!ent->key || !ent->value) {
			ht->keyfreefunc(ent->key, ht->mp);
			ht->valuefreefunc(ent->value, ht->mp);
			sh_free(ent, ht->mp);
			rv = -1;
			goto out;
		}
		rv = 0;
	}

	if (!sh_llist_append(ht->buckets[idx], ent)) {
		sh_free(ent, ht->mp);
		rv = -1;
		goto out;
	}

  out:
	ht->size += rv == 0 ?  1 : 0;
	return rv;

}

/* get the iterator after cur */
sh_ht_iterator_t * sh_ht_next(sh_ht_iterator_t *cur) {
	sh_ht_iterator_t *rv = NULL;
	sh_llist_iterator_t *lit = sh_llist_next(cur->lit);
	if (lit) {
		cur->lit = lit;
		rv = cur;
	} else {
		/* is there another bucket from here? */
		rv = sh_ht_from(cur->ht, cur->curbuck);
		free_ht_iterator(cur);
	}
	return rv;
}

/* puts the key into key (shallow ptr value only) and its length in keylen. */
void sh_ht_key(sh_ht_iterator_t *it, void **key, size_t *keylen) {
	struct sh_ht_entry *ent = sh_llist_payload(it->lit);	
	*key = ent->key;
	*keylen = ent->keylen;
}

/* puts the value into value (shallow ptr value only) and its length in vallen. */
void sh_ht_value(sh_ht_iterator_t *it, void **val, size_t *vallen) {
	struct sh_ht_entry *ent = sh_llist_payload(it->lit);	
	*val = ent->value;
	*vallen = ent->vallen;
}

/* a combination of the above two functions */
void sh_ht_kv(sh_ht_iterator_t *it, void **key, size_t *keylen, void **val, size_t *vallen) {
	struct sh_ht_entry *ent = sh_llist_payload(it->lit);	
	*key = ent->key;
	*keylen = ent->keylen;
	*val = ent->value;
	*vallen = ent->vallen;
}

/* get the item at key or NULL if it does not exist */
sh_ht_iterator_t * sh_ht_get(sh_ht_t *ht, void *key, size_t keylen) {
	struct sh_ht_iterator *rv = NULL;
	size_t idx = ht->keyhashfunc(key, keylen) % ht->bucket_size;


	if (ht->buckets[idx]) {
		sh_llist_iterator_t *cur = sh_llist_begin(ht->buckets[idx]);
		for(;cur;cur=sh_llist_next(cur)) {
			struct sh_ht_entry *old = sh_llist_payload(cur);
			if (old->keylen == keylen &&
			    ht->keycmpfunc(old->key, old->keylen,
			                   key, keylen) == 0) {

				rv = sh_malloc(sizeof(*rv), ht->mp);
				if (rv) {
					rv->lit = cur;
					rv->curbuck = idx;
					rv->ht = ht;
				}
				break;
			}
		}
	}
	return rv;
}

/* remove the item at *it from *ht. */
void sh_ht_remove(sh_ht_t *ht, sh_ht_iterator_t *it) {
	struct sh_ht_entry *ent = sh_llist_payload(it->lit);
	ht->keyfreefunc(ent->key, ht->mp);
	ht->valuefreefunc(ent->key,  ht->mp);
	sh_free(ent, ht->mp);
	sh_llist_remove(ht->buckets[it->curbuck], it->lit);
	--(ht->size);
}

/* get ht size */
size_t sh_ht_size(sh_ht_t *ht) {
	return ht->size;
}


struct llist_foreach_arg {
	void (*func)(void *, size_t , void *, size_t , void *);
	void *userarg;
};

static void llist_foreach_hlp(void *item, void *userarg) {
	struct sh_ht_entry *ent = item;
	struct llist_foreach_arg *arg = userarg;
	arg->func(ent->key, ent->keylen, ent->value, ent->vallen, arg->userarg);
}

/* iterate through the ht and apply callback to the payloads */
void sh_ht_foreach(sh_ht_t *ht, void (*func)(void *key, size_t keylen, void *value, size_t valuelen, void *userarg), void *userarg) {
	struct llist_foreach_arg arg = { func, userarg } ;
	for(size_t i = 0; i < ht->bucket_size; ++i) {
		if (ht->buckets[i]) {
			sh_llist_foreach(ht->buckets[i], llist_foreach_hlp, &arg);
		}
	}
}


static void ht_ent_free(void * payload, void *arg) {
	struct sh_ht_entry *ent = payload;
	sh_ht_t *ht = arg;
	ht->keyfreefunc(ent->key, ht->mp);
	ht->valuefreefunc(ent->value, ht->mp);
	sh_free(ent, ht->mp);
}

/* destroy the ht. Apply freefuntions to all payloads */
void sh_ht_destroy(sh_ht_t **ht) {
	for(size_t i = 0; i < (*ht)->bucket_size; ++i) {
		if ((*ht)->buckets[i]) {
			sh_llist_destroy(& (*ht)->buckets[i], ht_ent_free, *ht);
		}
	}
	sh_free((*ht)->buckets, (*ht)->mp);
	sh_free(*ht, (*ht)->mp);
	*ht = NULL;
}

