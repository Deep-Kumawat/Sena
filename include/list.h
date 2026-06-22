#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct list_s {
    struct list_s *next; // Pointer to the next node in the list
    struct list_s *prev; // Pointer to the previous node in the list
} list_t;


/*
 * container_of - cast a member of a structure out to the containing structure
 * @ptr: the pointer to the member
 * @type: the type of the container struct
 * @member: the name of the member within the struct
 * 
 * Works by calculating the offset of the member within the struct and then
 * subtracting that offset from the pointer to the member, effectively giving
 * us a pointer to the containing struct.
 */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

void list_init(list_t *list) {
    list->next = list;
    list->prev = list;
}

bool list_is_empty(list_t *list) {
    return list->next == list;
}

bool list_insert_after(list_t *list, list_t *new_node) {
    if (!list || !new_node) {
        return false; // Invalid input
    }
    // list <-> new_node <-> list->next
    new_node->next = list->next;
    new_node->prev = list;
    list->next->prev = new_node;
    list->next = new_node;
    return true; // Insertion successful
}

bool list_remove(list_t *node) {
    if (!node || node->next == node || node->prev == node) {
        return false; // Invalid input or trying to remove a sentinel node
    }
    // node->prev <-> node->next
    node->prev->next = node->next;
    node->next->prev = node->prev;
    // Clear the removed node's pointers
    node->next = NULL;
    node->prev = NULL;
    return true; // Removal successful
}

#endif // LIST_H