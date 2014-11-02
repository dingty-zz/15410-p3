/**
* @file linked_list.c
*
* @brief This file provides library functions to manipulate a doubly linked list
*
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
*
*/

#include "linked_list.h"
#include "assert.h"
#define NULL 0


/** @brief The function to initialize the doubly linked list
 *
 *  @param l a pointer to the list to be initialized
 *  @return nothing
 */
void list_init(list *l)
{
    if (l == NULL) return;
    l -> head = NULL;
    l -> tail = NULL;
    l -> length = 0;
}

node *list_begin(list *l) {
    return l -> head;
}
node *list_end(list *l) {
    return l -> tail;
}

/** @brief The generic function to delete a specific node from a list
 *
 *         Requires that j is stored in the list
 *  @param a pointer to the list and a pointer to the node
 *  @return the node that's deleted
 */
node *list_delete(list *l, node *j)
{
    if (l == NULL || j == NULL) return NULL;
    if (j -> prev != NULL)
        j -> prev -> next = j -> next;
    else if (l -> head == j)
        l -> head = j -> next;
    else
        return NULL;

    if (j -> next != NULL)
        j -> next -> prev = j -> prev;
    else if (l -> tail == j)
        l -> tail = j -> prev;
    else
        return NULL;
    l -> length--;
    return j;
}

/** @brief The function to delete the first node from list
 *
 *  @param l a pointer to the list
 *  @return the first node that's deleted
 */
node *list_delete_first(list *l)
{
    return list_delete(l, l -> head);
}

/** @brief The function to delete the last node from the list
 *
 *  @param l a pointer to the list
 *  @return the last node that's deleted
 */
node *list_delete_last(list *l)
{
    return list_delete(l, l -> tail);
}

// /** @brief The generic function to search for a specific tid in a list
//  *
//  *  @param l the pointer to the node with that specific tid
//  *  @return the node that matches the tid
//  */
// node *list_search(list *l, )
// {
//     if (l == NULL) return NULL;
//     node *temp = l -> head;
//     while (temp)
//     {
//         if (temp -> tid == tid)
//             return temp;
//         temp = temp -> next;
//     }
//     return NULL;
// }

/** @brief Insert a node at the first place in the linked list
 *
 *  @param l a pointer to the list
 *  @param j a pointer to the node
 *  @return void
 */
void list_insert_first(list *l, node *j)
{
    if (l == NULL || j == NULL) return;
    /*Empty list*/
    if (l -> head == NULL)
    {
        l -> head = l -> tail = j;
        j -> prev = j -> next = NULL;
    }
    /*Insert to the first place*/
    else
    {
        l -> head -> prev = j;
        j -> next = l -> head;
        j -> prev = NULL;
        l -> head = j;
    }
    l -> length++;
}


/** @brief Insert a node at the last place in the linked list
 *
 *  @param l a pointer to the list
 *  @param j a pointer to the node
 *  @return void
 */
void list_insert_last(list *l, node *j)
{
    // lprintf("list_insert_last");
    if (l == NULL || j == NULL) return;
    /*Empty list*/
    if (l -> tail == NULL)
    {
        l -> head = l -> tail = j;
        j -> prev = j -> next = NULL;
    }
    /*Insert to the last place*/
    else
    {
        l -> tail -> next = j;
        j -> prev = l -> tail;
        j -> next = NULL;
        l -> tail = j;
    }
    l -> length++;
}
