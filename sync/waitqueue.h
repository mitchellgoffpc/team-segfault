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




 /* =============================== *

  		  Data Structures

 * =============================== */

struct ProcessDescriptor;
struct WaitQueueNode;
struct WaitQueue;

typedef struct ProcessDescriptor ProcessDescriptor;
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
  The WaitQueue struct keeps track of a single waitqueue and allows
  us to iterate over all the processes, or to dequeue just the next
  process.
*/

struct WaitQueue {
	Spinlock lock;
	LinkedListNode head;
};




 /* =============================== *

  		     Interface

 * =============================== */

void aquireSpinlock(Spinlock *lock);
void releaseSpinlock(Spinlock *lock);

void addToWaitQueue(WaitQueueNode *node, WaitQueue *head);
void sleepOnWaitQueue(ProcessDescriptor* process, WaitQueue *head);
void sleepOnWaitQueueWithOptions(ProcessDescriptor* process, WaitQueue *head, int exclusive);
void signalWaitQueue(WaitQueue *head, int exclusive);

void prepareToSleep(ProcessDescriptor *process);
int tryToWakeUp(WaitQueueNode *node);



#endif
