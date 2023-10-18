#include "list.h"
#include "mem.h"
#include <traindef.h>

#include "kern/log.h"

typedef struct ListNode ListNode;

struct List {
    ListNode* head;
    ListNode* tail;
    size_t size;
};

struct ListNode {
    ListNode* next;
    ListNode* prev;
    void* data;
};

List*
list_init(void)
{
    List* new_list = alloc(sizeof(List)); 
    *new_list = (List) {
        .head = 0,
        .tail = 0,
        .size = 0,
    };

    return new_list;
}

void
list_push_front(List* list, void* item)
{
    ListNode* node = alloc(sizeof(ListNode));
    *node = (ListNode) {
        .next = list->head,
        .prev = 0,
        .data = item,
    };

    if (list->head != 0) {
        (list->head)->prev = node;
    }

    list->head = node;
    // special case when list is empty
    if (list->size == 0) {
        list->tail = node;
    }

    ++(list->size);
}
void
list_push_back(List* list, void* item)
{
    ListNode* node = alloc(sizeof(ListNode));
    *node = (ListNode) {
        .next = 0,
        .prev = list->tail,
        .data = item,
    };

    if (list->tail != 0) {
        (list->tail)->next = node;
    }

    list->tail = node;

    // special case when list is empty
    if (list->size == 0) {
        list->head = node;
    }

    ++(list->size);
}

void*
list_pop_front(List* list)
{
    if (list->head == 0) return 0;

    ListNode* next = list->head->next;
    void* data = list->head->data;
    if (next != 0) {
        next->prev = 0;
    }
    free(list->head);

    list->head = next;
    --(list->size);

    // special case when list is empty
    if (list->size == 0) {
        list->tail = 0;
    }

    return data;
}

void*
list_pop_back(List* list)
{
    if (list->tail == 0) return 0;

    ListNode* prev = list->tail->prev;
    void* data = list->tail->data;
    if (prev != 0) {
        prev->next = 0;
    }
    free(list->tail);

    list->tail = prev;
    --(list->size);

    // special case when list is empty
    if (list->size == 0) {
        list->head = 0;
    }
    return data;
}

void*
list_peek_front(List* list)
{
    if (list->head == NULL) {
        // TODO temp use kernel panic
        PANIC("can't get head of empty list");
    }
    return list->head->data;
}

void*
list_peek_back(List* list)
{
    if (list->tail == NULL) {
        // TODO temp use kernel panic
        PANIC("can't get tail of empty list");
    }
    return list->tail->data;
}

size_t
list_len(List* list)
{
    return list->size;
}

// TODO can't use this, closures are evil
/*
void*
list_find(List* list, ListFindFn pred)
{
    ListNode* node = list->head;
    while (node != 0) {

        if (pred(node->data)) return node->data;
        
        node = node->next;
    }
    return 0;
}
*/

// removes first occurance of element
bool
list_remove(List* list, void* item)
{
    ListNode* node = list->head;
    while (node != 0) {
        if (node->data == item) {
            if (node == list->head) {
                list_pop_front(list);
            }
            else if (node == list->tail) {
                list_pop_back(list);
            }
            else {
                // If it's not the head or the tail, it's guaranteed to have something before and after it
                node->prev->next = node->next;
                node->next->prev = node->prev;
                void* data = node->data;
                free(node);
                --(list->size);
            }
            return true;
        }
        node = node->next;
    }
    return false;
}

#if 0
// TODO untested
bool
list_remove_at(List* list, usize index)
{
    ListNode* node = list->head;
    for (usize i = 0; i < index; ++i) {
        if (node == NULL) return false;     
        node = node->next;
    }

    if (node == list->head) {
        list_pop_front(list);
    }
    else if (node == list->tail) {
        list_pop_back(list);
    }
    else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        void* data = node->data;
        free(node);
        --(list->size);
    }

    return true;
}
#endif

void
list_deinit(List* list)
{
    // TODO free entire list
}

ListIter
list_iter(List* list) {
    return (ListIter) {
        .list = list,
        .node = list->head
    };
}

bool
listiter_next(ListIter* it, void** item)
{
    if (it->node == 0) return false;
    *item = it->node->data;
    it->node = it->node->next;
    return true;
}

#if 0
// TODO untested
// Insert item before item pointed to by iter
void
listiter_insert_before(ListIter* it, void* item)
{
    ListNode* new_node = alloc(sizeof(ListNode));
    *new_node = (ListNode) {
        .next = it->node->next,
        .prev = it->node->prev,
        .data = item,
    };
    
    // update next and previous node's pointers
    if (it->node->prev != 0) it->node->prev->next = new_node;
    if (it->node->next != 0) it->node->next->prev = new_node;

    // set head and tail accordinly
    if (it->node->prev == 0) it->list->head = new_node; 
    if (it->node->next == 0) it->list->tail = new_node; 

    ++(it->list->size);
}

// Delete item at listiter
// moves iterator to item after
void
listiter_delete_at(ListIter* it)
{
    if (it->node == 0) return;

    if (it->node->prev == 0) it->list->head = it->node->next;
    if (it->node->next == 0) it->list->tail = it->node->prev;

    if (it->node->prev != 0) it->node->prev->next = it->node->next;
    if (it->node->next != 0) it->node->next->prev = it->node->prev;

    it->node = it->node->next;

    --(it->list->size);
}
#endif
