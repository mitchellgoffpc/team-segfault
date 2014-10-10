/*
  File: waitqueue.h
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  			  Includes

 * =============================== */

#include <stdlib.h>

#include "../process/list.h"
#include "../process/process.h"
#include "waitqueue.h"




/* =============================== *

  		   Implementation

 * =============================== */

/*
  Some basic functionality for spinlocks. These are rather inefficient, and
  should only be used to ensure that the waitqueues are properly synchronized.
  
  Note: On Yalnix, putting a spinlock on wait queues is probably superfluous
  since interrupts are disabled while in kernel mode. However, including them
  is good practice in case we ever want to port the OS to another system.
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
  Macros and functions to create an initialize waitqueues
*/

// Statically initialize a new waitqueue
#define waitQueue(name) { 0, linkedListNode(name) }

// Dynamically initialize a new waitqueue
#define waitQueueInit(name) \
  	(name)->lock = 0; \
  	linkedListNodeInit(&name->node)

// Create a new waitqueue variable
#define newWaitQueue(name) \
	WaitQueue name = waitQueue(name)



// Statically initialize a new waitqueue node
#define waitQueueNode(name, process) { 1, &tryToWakeUp, process, linkedListNode(name) }

// Dynamically initialize a new waitqueue node
#define waitQueueNodeInit(name, process) \
	(name)->is_exclusive = 0; \
	(name)->prepareToWakeUp = &tryToWakeUp; \
	(name)->process = (process); \
	linkedListNodeInit(&name->node)

// Create a new waitqueue node variable
#define newWaitQueueNode(name) \
	WaitQueueNode name = waitQueueNode(name)




/*
  Allow a process to add itself to a waitqueue, then sleep until it is woken up.
  Note: For a more robust implementation, we would need to add a memory barier
  before releasing the spinlock. However, this opens a whole can of worms that
  we don't really need to worry about, so I'm saving that project for another time.
*/

// Add a waitqueue node to a waitqueue
void addToWaitQueue(WaitQueueNode *node, WaitQueue *head) {
	aquireSpinlock(&head->lock);

	// If this node is exclusive, add it to the back of the queue. Otherwise,
	// if it's non-exclusive, add it to the front so it will be woken up first.
	node->is_exclusive ?
		addLastNode(&node->node, &head->head) :
		addFirstNode(&node->node, &head->head);

	releaseSpinlock(&head->lock);
	putProcessToSleep(node->process);
}

// Add an exclusive process to a particular waitqueue
void sleepOnWaitQueue(ProcessDescriptor* process, WaitQueue *head) {
	sleepOnWaitQueueWithOptions(process, head, 1); // defaults to exclusive
}

// Create a waitqueue node for a process and add that node to a waitqueue
void sleepOnWaitQueueWithOptions(ProcessDescriptor* process, WaitQueue *head, int exclusive) {
	WaitQueueNode *node = (WaitQueueNode*) malloc(sizeof(WaitQueueNode));
	waitQueueNodeInit(node, process);
	addToWaitQueue(node, head);
}




/*
  Go through a waitqueue and wake up any eligable processes.

  exclusive: Determines whether all exclusive processes should be woken up
  			 (broadcast-style), or only one (signal-style).
*/

void signalWaitQueue(WaitQueue *head, int exclusive) {
	aquireSpinlock(&head->lock);
	WaitQueueNode *current;

	// If we're doing an exclusive wakeup, start dequeueing nodes until we
	// successfully wake up an exclusive process. Otherwise, just wake up everthing.
	forEachElement(current, &head->head, node) {
		removeFirstNode(&head->head);
		int succeeded = current->prepareToWakeUp(current);
		if (succeeded && current->is_exclusive && exclusive)
			break;
	}

	releaseSpinlock(&head->lock);
}





/*
  Functions to wake up a process
*/

void putProcessToSleep(ProcessDescriptor *process) {
	process->state = PROCESS_WAITING;
	// schedule();
}

int tryToWakeUp(WaitQueueNode *node) {
	return 1;
}
