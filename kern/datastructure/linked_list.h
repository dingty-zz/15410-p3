/**
* @file linked_list.h
*
* @brief Defines the list node, list header data structure and provides some
*        library functions on the list
*        This is a doubly linked list where the header struct is stored on the stack
*        while nodes are allocated on heap. The application needs to encapsulate the
*        data pointed using the make_node function provided below.
*
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
*
*/

#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *) 0)->MEMBER)
#define list_entry(LIST_ELEM, STRUCT, MEMBER)    \
    ((STRUCT *) ((uint8_t *) LIST_ELEM    \
                 - offsetof (STRUCT, MEMBER)))


/* Generic list node */
typedef struct node_t
{
    struct node_t   *prev;  // Previous node
    struct node_t   *next;  // Next node
} node;


// Generic doubly linked list structure
typedef struct list_t
{
    int     length;     // Length of the list
    node    *head;      // Header element
    node    *tail;      // Tail element
} list;


// Some generic list functions
node *list_begin(list *l);
node *list_end(list *l);

void list_init(list *l);
node *list_delete(list *l, node *n);  // generic delete function
node *list_delete_first(list *l);
node *list_delete_last(list *l);
void list_insert_first(list *l, node *j);
void list_insert_last(list *l, node *j);

#endif /* _LINKED_LIST_H */
