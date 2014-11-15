/*
  File: waitqueue.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  			  Includes

 * =============================== */

#include <stdlib.h>
#include <errno.h>

#include "../core/list.h"
#include "../process/process.h"
#include "sync.h"




/* =============================== *

  		   Implementation

 * =============================== */

/*
  Some basic functionality for spinlocks.

  Note: On Yalnix, using spinlocks inside the kernel is probably superfluous
  since interrupts are disabled while in kernel mode.
*/

void aquireSpinlock(Spinlock *lock) {
	while (__sync_lock_test_and_set(lock, 1)) {
		while (*lock);
	}
}

void releaseSpinlock(Spinlock *lock) {
	__sync_lock_release(lock);
}




/*
  Allow a process to add itself to a waitqueue, then sleep until it is woken up.
*/

// Add a waitqueue node to a waitqueue
void addNodeToWaitQueue(WaitQueueNode *node, WaitQueue *head) {

	// If this node is exclusive, add it to the back of the queue. Otherwise,
	// if it's non-exclusive, add it to the front so it will be woken up first.
	node->is_exclusive ?
		addLastNode(&node->node, &head->head) :
		addFirstNode(&node->node, &head->head);
}

// Add an exclusive process to a particular waitqueue
int sleepOnWaitQueue(WaitQueue *head) {
	return sleepOnWaitQueueWithOptions(head, 1); // defaults to exclusive
}

// Create a waitqueue node for a process and add that node to a waitqueue
int sleepOnWaitQueueWithOptions(WaitQueue *head, int exclusive) {
	WaitQueueNode *node = (WaitQueueNode*) malloc(sizeof(WaitQueueNode));
	if (!node) return errno;

	// Set up a new waitqueue node	
	waitQueueNodeInit(node);
	node->is_exclusive = exclusive;
	getCurrentProcess()->waitqueue = node;

	// Add the node to the waitqueue, then put the process to sleep.
	addNodeToWaitQueue(node, head);
	putProcessToSleep();

	return 0;
}




/*
  Signal the waitqueue that another process can be woken up.

  exclusive: Determines whether all exclusive processes should be woken up
  			 (broadcast-style), or only one (signal-style).
*/

void signalWaitQueue(WaitQueue *head) {
	signalWaitQueueWithOptions(head, 1); // defaults to exclusive
}

void signalWaitQueueWithOptions(WaitQueue *head, int wakeup_is_exclusive) {
	WaitQueueNode *current;

	// If we're doing an exclusive wakeup, start dequeueing nodes until we
	// successfully wake up an exclusive process. Otherwise, just wake up everthing.
	forEachElement(current, &head->head, node) {
		removeFirstNode(&head->head);
		
		int node_is_exclusive = current->is_exclusive;
		current->prepareToWakeUp(current);
		
		if (node_is_exclusive && wakeup_is_exclusive) break;
	}
}




/*
  Functions to put to sleep and wake up a process
*/

void putProcessToSleep() {
	getCurrentProcess()->state = PROCESS_WAITING;
	schedule();
}

int wakeUpProcess(WaitQueueNode *node) {
	node->process->state = PROCESS_RUNNING;
	free(node);
	return 0;
}
