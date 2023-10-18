#ifndef __TRAINSTD_LIST_H__
#define __TRAINSTD_LIST_H__

/* doubly linked list implementation */

#include <traindef.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct List List;
typedef struct ListNode ListNode;
typedef struct ListIter ListIter;
struct ListIter {
    List* list;
    ListNode* node;
};

List* list_init(void);
void list_deinit(List* list);

void list_push_front(List* list, void* item);
void list_push_back(List* list, void* item);

void* list_pop_front(List* list);
void* list_pop_back(List* list);

void* list_peek_front(List* list);
void* list_peek_back(List* list);

// Search the list for an item and remove it. Returns true if the item was in the list, false otherwise.
// bool list_remove(List* list, void* item);

/* typedef bool(*ListFindFn)(void*) ; */
/* void* list_find(List* list, ListFindFn pred); */

size_t list_len(List* list);

bool list_remove(List* list, void* item);
// bool list_remove_at(List* list, usize index);

// TODO list iter might be a bit too overkill since an alloc is necessary
// could instead make iteration by passing a function List* and ListNode*
ListIter list_iter(List* list);
bool listiter_next(ListIter* it, void** item);

// void listiter_insert_before(ListIter* it, void* item);
//void listiter_delete_at(ListIter* it);

#endif // __TRAINSTD_LIST_H__
