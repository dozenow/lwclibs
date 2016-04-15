#ifndef SH_LLIST_H
#define SH_LLIST_H

#include <stddef.h>

struct sh_llist;
struct sh_memory_pool;

typedef struct sh_llist sh_llist_t;

struct sh_llist_iterator; /* note, iterators are not guaranteed to be valid if the list is mutated */
typedef struct sh_llist_iterator sh_llist_iterator_t;


extern sh_llist_t *sh_llist_create(struct sh_memory_pool *mp);

/* return an iterator to the beginning of the list. Returns item iterator */
extern sh_llist_iterator_t * sh_llist_begin(sh_llist_t *list);

/* append an item to the list. Returns item iterator */
extern sh_llist_iterator_t * sh_llist_append(sh_llist_t *list, void *item);

/* prepend an item to the list. Returns item iterator */
extern sh_llist_iterator_t * sh_llist_prepend(sh_llist_t *list, void *item);

/* insert an item after *it. Returns item iterator */
extern sh_llist_iterator_t * sh_llist_insert(sh_llist_iterator_t *it, void *item);

/* get the iterator after cur */
extern sh_llist_iterator_t * sh_llist_next(sh_llist_iterator_t *cur);

/* remove the item at *it from *list. Returns element after it */
extern sh_llist_iterator_t * sh_llist_remove(sh_llist_t *list, sh_llist_iterator_t *it);

/* get list size */
extern size_t sh_llist_size(sh_llist_t *list);

/* get it payload */
extern void * sh_llist_payload(sh_llist_iterator_t *it);

/* iterate through the list and apply callback to the payloads */
extern void sh_llist_foreach(sh_llist_t *list, void (*func)(void *item, void *userarg), void *userarg);

/* destroy the list. Apply freefunc to all payloads */
extern void sh_llist_destroy(sh_llist_t **list, void (*freefunc)(void*, void*arg), void *userarg);


#endif
