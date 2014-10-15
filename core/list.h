/*
  File: list.h
  Date: 10/6/2014
  Author: Mitchell Goff
*/

#ifndef __YALNIX_LIST_H__
#define __YALNIX_LIST_H__



/* =============================== *

  	          Macros

 * =============================== */

// A linked list node
struct LinkedListNode {
	struct LinkedListNode *next, *prev;
};

typedef struct LinkedListNode LinkedListNode;



/*
  These are some macros to get an element containing a linked list node. They're
  a bit magicky, but extremely useful for performing operations on the element
  itself, rather than the linked list node.
  Credit: the linux kernel source
*/

// Magic macro to get the position of a member in a larger struct
#define offsetof(type, member) ((size_t) &((type *)0)->member)

// Magic macro to get the structure containing a node
#define containerOf(ptr, type, member) ({ \
	const typeof( ((type *)0)->member ) *__mptr = (ptr); \
	(type *) ((char *)__mptr - offsetof(type, member) ); })

// Just an alias for containerOf
#define elementForNode(ptr, type, member) \
	containerOf(ptr, type, member)



/*
  Some macros and functions to provide basic linked list functionality.
*/

// Statically initialize a new linked list node
#define linkedListNode(name) { &(name), &(name) }

// Dynamically initialize a new linked list node
#define linkedListNodeInit(name) \
  	(name)->prev = (name); \
  	(name)->next = (name)

// Create a new linked list node variable
#define newLinkedListNode(name) \
	LinkedListNode name = linkedListNode(name)



// Test whether a list is empty
inline int listIsEmpty(LinkedListNode *head) {
	return head->next == head;
}

// Insert a new node into the list
static inline void insertNode(LinkedListNode *node, LinkedListNode *head) {
	head->next->prev = node;
	node->next = head->next;
	node->prev = head;
	head->next = node;
}

// Remove a node from the list
static inline void removeNode(LinkedListNode *node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

// Splice list $second into list $first
static inline void spliceLinkedLists(LinkedListNode *second, LinkedListNode *first) {
	if (!listIsEmpty(second)) {
		second->prev->next = first;
		second->next->prev = first->prev;
		first->prev->next = second->prev;
		first->prev = second->next;
	}
}



/*
  Some macros and functions to simplify the job of inserting and removing nodes
  at the beginning and end of a linked list
*/

// Add a node to the beginning of the list
static inline void addFirstNode(LinkedListNode *node, LinkedListNode *head) {
	insertNode(node, head);
}

// Add a node to the beginning of the list
static inline void addLastNode(LinkedListNode *node, LinkedListNode *head) {
	insertNode(node, head->prev);
}

// Remove the node at the beginning of the list
static inline LinkedListNode* removeFirstNode(LinkedListNode *head) {
	LinkedListNode* node = head->next;
	removeNode(node);
	return node;
}

// Remove the node at the end of the list
static inline LinkedListNode* removeLastNode(LinkedListNode *head) {
	LinkedListNode* node = head->prev;
	removeNode(node);
	return node;
}



/*
  Some aliases to give a linked list queue-like and stack-like terminology
*/

// Enqueue a node to the end of the list
#define enqueueNode(node, head) \
	addLastNode(node, head)

// Dequeue a node from the beginning of the list
#define dequeueNode(head) \
	removeFirstNode(head)

// Push a node to the end of the list
#define pushNode(node, head) \
	addLastNode(node, head)

// Pop a node from the end of the list
#define popNode(head) \
	removeLastNode(head)



/*
  Some macros to allow element-wise access to list nodes
*/

// Insert an element into the list
#define insertElement(element, member, head) \
	insertNode(&(element)->member, head)

// Remove an element from the list
#define removeElement(element, member) \
	removeNode(&(element)->member)


// Insert an element at the beginning of the list
#define addFirstElement(element, member, head) \
	addFirstNode(&(element)->member, head)

// Insert an element at the end of the list
#define addLastElement(element, member, head) \
	addLastNode(&(element)->member, head)

// Remove an element from the beginning of the list
#define removeFirstElement(type, member, head) \
	elementForNode(removeFirstNode(head), type, member)

// Remove an element from the end of the list
#define removeLastElement(type, member, head) \
	elementForNode(removeLastNode(head), type, member)


// Enqueue an element to the end of the list
#define enqueueElement(element, member, head) \
	addLastElement(element, member, head)

// Dequeue an element from the beginning of the list
#define dequeueElement(type, member, head) \
	removeFirstElement(type, member, head)

// Push an element to the end of the list
#define pushElement(element, member, head) \
	addLastElement(element, member, head)

// Pop an element from the end of the list
#define popElement(type, member, head) \
	removeLastElement(type, member, head)



/*
  Some macros to iterate over every node in a list.
  Credit: the linux kernel source
*/

// Iterate over every node in the list
#define forEachNode(pos, head) \
	for (LinkedListNode* pos = (head)->next; \
		pos != (head); \
		pos = pos->next)

// Iterate backwards over every node in the list
#define forEachNodeReversed(pos, head) \
	for (LinkedListNode* pos = (head)->prev; \
		pos != (head); \
		pos = pos->prev)

// Iterate over every element in the list
// Node: an element is any struct containing a LinkedListNode
#define forEachElement(pos, head, member) \
	for (pos = elementForNode((head)->next, typeof(*pos), member); \
		&pos->member != (head); \
		pos = elementForNode(pos->member.next, typeof(*pos), member))

// Iterate backwards over every element in the list
#define forEachElementReversed(pos, head, member) \
	for (pos = elementForNode((head)->prev, typeof(*pos), member); \
		&pos->member != (head); \
		pos = elementForNode(pos->member.prev, typeof(*pos), member))



#endif
