#ifndef SHARED_HT_H
#define SHARED_HT_H

#include <stddef.h>

struct sh_ht;

struct sh_memory_pool;

typedef struct sh_ht sh_ht_t;

struct sh_ht_iterator; /* note, iterators are not guaranteed to be valid if the HT is mutated */
typedef struct sh_ht_iterator sh_ht_iterator_t;


/* create takes a comparison function, a copy function, a free
 * function, and a memory pool. Any items passed into update will have
 * copy called on them. All items will have their freefuncs called on
 * them eventually. If the buckets are resized, they may be copied and
 * freed.
 */
extern sh_ht_t *sh_ht_create(int (*cmpfunc)(void*, size_t, void*, size_t),
                             size_t (*keyhashfunc)(void*,size_t),
                             void *(*keycopyfunc)(void*,size_t, struct sh_memory_pool *),
                             void *(*datacopyfunc)(void*,size_t, struct sh_memory_pool *), 
                             void (*keyfreefunc)(void*, struct sh_memory_pool *),
                             void (*datafreefunc)(void*, struct sh_memory_pool *),
                             struct sh_memory_pool *);

/* call when you're done with an iterator */
extern void free_ht_iterator(sh_ht_iterator_t *it);

/* return an iterator to the beginning of the ht. Returns item iterator */
extern sh_ht_iterator_t * sh_ht_begin(sh_ht_t *ht);

/* add/update an item to the ht. Returns 1 if the item already existed and 0 otherwise. -1 on error */
extern int sh_ht_update(sh_ht_t *ht, void *key, size_t keylen, void *data, size_t datalen);

/* get the iterator after cur. Modifies or frees cur */
extern sh_ht_iterator_t * sh_ht_next(sh_ht_iterator_t *cur);

/* puts the key into key (shallow ptr value only) and its length in keylen. */
extern void sh_ht_key(sh_ht_iterator_t *it, void **key, size_t *keylen);

/* puts the value into value (shallow ptr value only) and its length in vallen. */
extern void sh_ht_value(sh_ht_iterator_t *it, void **val, size_t *vallen);

/* a combination of the above two functions */
extern void sh_ht_kv(sh_ht_iterator_t *it, void **key, size_t *keylen, void **val, size_t *vallen); 

/* get the item at key or NULL if it does not exist */
extern sh_ht_iterator_t * sh_ht_get(sh_ht_t *ht, void *key, size_t keylen);

/* remove the item at *it from *ht. */
extern void sh_ht_remove(sh_ht_t *ht, sh_ht_iterator_t *it);

/* get ht size */
extern size_t sh_ht_size(sh_ht_t *ht);

/* iterate through the ht and apply callback to the payloads */
extern void sh_ht_foreach(sh_ht_t *ht, void (*func)(void *key, size_t keylen, void *value, size_t valuelen, void *userarg), void *userarg);

/* destroy the ht. Apply freefunc to all payloads */
extern void sh_ht_destroy(sh_ht_t **ht);

#endif
