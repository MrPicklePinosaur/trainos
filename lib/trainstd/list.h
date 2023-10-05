#ifndef __TRAINSTD_LIST_H__
#define __TRAINSTD_LIST_H__

/* doubly linked list implementation */

#include <stddef.h>
#include <stdbool.h>

typedef struct List List;
typedef struct ListIter ListIter;

List* list_init(void);
void list_deinit(List* list);

void list_push_front(List* list, void* item);
void list_push_back(List* list, void* item);

void* list_pop_front(List* list);
void* list_pop_back(List* list);

void* list_peek_front(List* list);
void* list_peek_back(List* list);

// Search the list for an item and remove it. Returns true if the item was in the list, false otherwise.
bool list_remove(List* list, void* item);

/* typedef bool(*ListFindFn)(void*) ; */
/* void* list_find(List* list, ListFindFn pred); */

size_t list_len(List* list);

ListIter* list_iter(List* list);
void* listiter_next(ListIter* it);
bool listiter_end(ListIter* it);
void listiter_delete(ListIter* it);

#endif // __TRAINSTD_LIST_H__
