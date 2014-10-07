/* Tests for list.h */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../list.h"


struct ListElement {
	char element[40];
	struct LinkedListNode node;
};

#define newElement(var, name) \
	struct ListElement var; \
	strcpy(var.element, name); \
	var.node.next = &var.node; \
	var.node.prev = &var.node


void testListNodes() {
	newLinkedListNode(head);

	// Try queueing a list node
	newLinkedListNode(queue);
	addFirstNode(&queue, &head);
	assert(head.next == &queue);

	// Try pushing a list node
	newLinkedListNode(push);
	addLastNode(&push, &head);
	assert(head.prev == &push);

	// Try inserting a list node after $queue
	newLinkedListNode(next);
	insertNode(&next, &queue);
	assert(queue.next == &next);


	// Try iterating over each node
	struct LinkedListNode* pos; int i=0;
	forEachNode(pos, &head) {
		printf("Visiting node %d\n", i);
		i++;
	}


	// Try dequeuing a list node
	assert(removeFirstNode(&head) == &queue);
	assert(head.next == &next);

	// Try popping a node
	assert(removeLastNode(&head) == &push);
	assert(head.prev == &next);

	// Try removing a node
	removeNode(&next);
	assert(head.next == &head && head.prev == &head);
}



void testListElements() {
	newLinkedListNode(head);

	// Try queueing a list node
	newElement(queue, "Enqueued element");
	// struct ListElement queue;
	// strcpy(queue.element, "Enqueued element");
	// queue.node.next = &queue.node;
	// queue.node.prev = &queue.node;

	addFirstElement(&queue, node, &head);
	assert(head.next == &queue.node);

	// Try pushing a list node
	newElement(push, "Pushed element");
	addLastElement(&push, node, &head);
	assert(head.prev == &push.node);

	// Try inserting a list node after $queue
	newElement(next, "Inserted element");
	insertElement(&next, node, &queue.node);
	assert(queue.node.next == &next.node);


	// Try iterating over each node
	struct ListElement* pos;
	forEachElement(pos, &head, node) {
		printf("Visiting: %s\n", pos->element);
	}


	// Try dequeuing a list node
	assert(removeFirstElement(ListElement, node, &head) == &queue);
	assert(head.next == &next.node);

	// Try popping a node
	assert(removeLastElement(ListElement, node, &head) == &push);
	assert(head.prev == &next.node);

	// Try removing a node
	removeElement(&next, node);
	assert(head.next == &head && head.prev == &head);
}



int main() {
	testListNodes();
	testListElements();

	printf("All tests passed!\n");
	return 0;
}