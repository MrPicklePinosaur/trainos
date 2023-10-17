#include "list.h"
#include "trainstd.h"

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

struct ListIter {
    List* list;
    ListNode* node;
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

void
list_deinit(List* list)
{
    // TODO free entire list
}

ListIter*
list_iter(List* list) {
    ListIter* it = alloc(sizeof(ListIter));
    *it = (ListIter) {
        .list = list,
        .node = list->head
    };
    return it;
}

void*
listiter_next(ListIter* it)
{
    if (it->node == 0) return 0;
    void* data = it->node->data;
    it->node = it->node->next;
    return data;
}

bool
listiter_end(ListIter* it)
{
    return it->node == 0;
}

void
listiter_deinit(ListIter* it)
{
    free(it);
}

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
}

// Delete item at listiter
// THE USER MUST ADVANCE THE LIST ITERATOR THEMSELVES
void
listiter_delete_at(ListIter* it)
{
    if (it->node == 0) return;

    if (it->node->prev == 0) it->list->head = it->node->next;
    if (it->node->next == 0) it->list->tail = it->node->prev;

    if (it->node->prev != 0) it->node->prev = it->node->next;
    if (it->node->next != 0) it->node->next = it->node->prev;
}
