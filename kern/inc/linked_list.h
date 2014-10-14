 /**
 * @file linked_list.h
 *
 * @brief Defines the list node, list header data structure and provides some
 *		  library functions on the list 
 *		  This is a doubly linked list where the header struct is stored on the stack
 *	      while nodes are allocated on heap. The application needs to encapsulate the 
 *		  data pointed using the make_node function provided below.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

/* Generic list node */
typedef struct node_t
{
	void 	*data;  // thread_t when the list stores tcb, and null when used
					// in mutex, cond_var, etc.
	int 	tid;  	// thread's tid
	struct node_t	*prev;	// Previous node
	struct node_t	*next;	// Next node
} node;


// very very generic doubly linked list
typedef struct list_t
{
	int 	length; 	// Length of the list
	void 	*head;		// Header element
    void 	*tail;		// Tail element
} list;


// Some generic list functions
void list_init(list *l);
node *list_delete(list *l, node *n);  // generic delete function
node *list_delete_id(list *l, int tid);  // delete this node with corresponding tid
node *list_delete_first(list *l);
node *list_delete_last(list *l);
void list_insert_first(list *l, node *j);
void list_insert_last(list *l, node *j);	
node *list_search(list *l, int tid); // generic search function
node *make_node(void *data, int tid);

#endif /* _LINKED_LIST_H */
