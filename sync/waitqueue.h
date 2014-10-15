/*
  File: waitqueue.h
  Date: 10/6/2014
  Author: Mitchell Goff
*/

#ifndef __YALNIX_WAITQUEUE_H__
#define __YALNIX_WAITQUEUE_H__



/* =============================== *

  			  Includes

 * =============================== */

#include "../process/list.h"
#include "../process/process.h"




 /* =============================== *

  		  Data Structures

 * =============================== */

struct WaitQueueNode;
struct WaitQueue;

typedef struct WaitQueueNode WaitQueueNode;
typedef struct WaitQueue WaitQueue;

typedef volatile int Spinlock;
typedef int (*WaitQueueHandler) (WaitQueueNode*);




/*
  The WaitQueueNode struct allows processes to add themselves to a
  waitqueue and get notified when some event becomes true.

  is_exclusive:   Determines whether the process is exclusive or not
  wakeup_handler: The function to run when the process gets off the waitqueue
  process:		  The process to add to the waitqueue
  node:			  A linked list node for the waitqueue to hook onto
*/

struct WaitQueueNode {
	unsigned int is_exclusive;
	WaitQueueHandler prepareToWakeUp;
	ProcessDescriptor *process;
	LinkedListNode node;
};




/*
  The WaitQueue struct keeps track of a single waitqueue and allows us to
  iterate over all the processes, or to dequeue just the next process.
*/

struct WaitQueue {
	LinkedListNode head;
};




 /* =============================== *

  		      Macros

 * =============================== */

/*
  Macros and functions to create an initialize waitqueues
*/

// Statically initialize a new waitqueue
#define waitQueue(name) { linkedListNode(name.head) }

// Dynamically initialize a new waitqueue
#define waitQueueInit(name) \
  	linkedListNodeInit(&name->node)

// Create a new waitqueue variable
#define newWaitQueue(name) \
	WaitQueue name = waitQueue(name)



// Statically initialize a new waitqueue node
#define waitQueueNode(name, process) { 1, &wakeUpProcess, process, linkedListNode(name.node) }

// Dynamically initialize a new waitqueue node
#define waitQueueNodeInit(name, process) \
	(name)->is_exclusive = 0; \
	(name)->prepareToWakeUp = &wakeUpProcess; \
	(name)->process = (process); \
	linkedListNodeInit(&name->node)

// Create a new waitqueue node variable
#define newWaitQueueNode(name) \
	WaitQueueNode name = waitQueueNode(name)




 /* =============================== *

  		     Interface

 * =============================== */

void aquireSpinlock(Spinlock *lock);
void releaseSpinlock(Spinlock *lock);

void addToWaitQueue(WaitQueueNode *node, WaitQueue *head);
int sleepOnWaitQueue(ProcessDescriptor* process, WaitQueue *head);
int sleepOnWaitQueueWithOptions(ProcessDescriptor* process, WaitQueue *head, int exclusive);
void signalWaitQueue(WaitQueue *head);
void signalWaitQueueWithOptions(WaitQueue *head, int exclusive);

void putProcessToSleep(ProcessDescriptor *process);
int wakeUpProcess(WaitQueueNode *node);



#endif
