#include "shared_llist.h"
#include "shared_malloc.h"
#include <stdlib.h>



struct sh_llist_iterator {
	struct sh_memory_pool *mp;
	void * payload;
	struct sh_llist_iterator *next;
};

struct sh_llist {
	struct sh_memory_pool *mp;
	sh_llist_iterator_t *head;
	size_t size;
};


sh_llist_t *sh_llist_create(struct sh_memory_pool *mp) {
	sh_llist_t *l = sh_calloc(1, sizeof(*l), mp);
	if (!l)
		return l;

	l->mp = mp;
	return l;
}

sh_llist_iterator_t * sh_llist_begin(sh_llist_t *list) {
	return list->head;
}

sh_llist_iterator_t * sh_llist_append(sh_llist_t *list, void *item) {
	sh_llist_iterator_t *it = sh_malloc(sizeof(*it), list->mp);
	if (!it)
		return NULL;
	it->payload = item;
	it->next = NULL;
	it->mp = list->mp;

	if (!list->head) {
		list->head = it;
	} else {
		sh_llist_iterator_t *cur = list->head;
		sh_llist_iterator_t *last = cur;
		for(;cur; cur = cur->next) {
			last = cur;
		}
		last->next = it;
	}
	list->size += 1;
	return it;
}

sh_llist_iterator_t * sh_llist_prepend(sh_llist_t *list, void *item) {
	sh_llist_iterator_t *it = sh_malloc(sizeof(*it), list->mp);
	if (!it)
		return NULL;
	it->payload = item;
	it->next = list->head;
	it->mp = list->mp;
	list->head = it;
	list->size += 1;
	return it;
}

sh_llist_iterator_t * sh_llist_insert(sh_llist_iterator_t *it, void *item) {
	sh_llist_iterator_t *cur = sh_malloc(sizeof(*cur), it->mp);
	if (!cur)
		return NULL;
	cur->payload = item;
	cur->next = it->next;
	cur->mp = it->mp;
	it->next = cur;
	return cur;
}


sh_llist_iterator_t * sh_llist_next(sh_llist_iterator_t *cur) {
	return cur ? cur->next : NULL;
}

sh_llist_iterator_t *sh_llist_remove(sh_llist_t *list, sh_llist_iterator_t *it) {
	sh_llist_iterator_t *prev = NULL;
	sh_llist_iterator_t *next = NULL;
	sh_llist_iterator_t *cur = list->head;
	if (it == list->head) {
		next = it->next;
		list->head = it->next;
	} else {
		for(;cur != it; cur = cur->next) {
			prev = cur;
		}
		prev->next = next = it->next;
	}


	sh_free(it, it->mp);
	list->size--;
	return next;
}

size_t sh_llist_size(sh_llist_t *list) {
	return list->size;
}

void * sh_llist_payload(sh_llist_iterator_t *it) {
	return it->payload;
}

void sh_llist_foreach(sh_llist_t *list, void (*func)(void *item, void *userarg), void *userarg) {
	sh_llist_iterator_t *cur = list->head;
	for(;cur;cur = cur->next) {
		func(cur->payload, userarg);
	}
}

void sh_llist_destroy(sh_llist_t **list, void (*freefunc)(void*, void*), void *arg) {
	sh_llist_iterator_t *last = NULL;
	sh_llist_iterator_t *cur = (*list)->head;
	for(;cur;cur = cur->next) {
		if (freefunc) {
			freefunc(cur->payload, arg);
		}
		if (last) {
			sh_free(last, last->mp);
			last = NULL;
		}
		last = cur;
	}

	if (last) {
		sh_free(last, last->mp);
		last = NULL;
	}

	sh_free(*list, (*list)->mp);
	*list = NULL;
}

