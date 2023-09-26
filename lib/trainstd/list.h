#ifndef __TRAINSTD_LIST_H__
#define __TRAINSTD_LIST_H__

/* doubly linked list implementation */

#include <stddef.h>

typedef struct List List;

List* list_init(void);
void list_deinit(List* list);

void list_push_front(List* list, void* item);
void list_push_back(List* list, void* item);

void* list_pop_front(List* list);
void* list_pop_back(List* list);

void* list_peek_front(List* list);
void* list_peek_back(List* list);

size_t list_len(List* list);


#endif // __TRAINSTD_LIST_H__
