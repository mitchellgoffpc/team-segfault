/*
  File: sync.h
  Date: 10/8/2014
  Author: Mitchell Goff
*/

#ifndef __YALNIX_SYNC_H__
#define __YALNIX_SYNC_H__



/* =============================== *

  			  Includes

 * =============================== */

#include "../core/list.h"
#include "../process/process.h"





/* =============================== *

  		   Data Structures

 * =============================== */

struct Resource;
struct WaitQueueNode;
struct WaitQueue;
struct Mutex;
struct CondVar;

typedef struct Resource Resource;
typedef struct WaitQueueNode WaitQueueNode;
typedef struct WaitQueue WaitQueue;
typedef struct Mutex Mutex;
typedef struct CondVar CondVar;

typedef volatile int Spinlock;
typedef int (*WaitQueueHandler) (WaitQueueNode*);

extern LinkedListNode resource_head;




/*
  An enum to indicate which type of resource is being pointed to
*/

enum ResourceType {
    RESOURCE_MUTEX,
    RESOURCE_CVAR,
    RESOURCE_PIPE,
};




/*
  The Resource struct allows us to keep references to all synchronization resources
  in a single linked list
*/

struct Resource {
    unsigned int id;
    enum ResourceType type;
    void *location;
    LinkedListNode node;
};




/*
  The WaitQueueNode struct allows processes to add themselves to a
  waitqueue and get notified when some event becomes true.

  is_exclusive:   Determines whether the process is exclusive or not
  wakeup_handler: The function to run when the process gets off the waitqueue
  process:      The process to add to the waitqueue
  node:       A linked list node for the waitqueue to hook onto
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



/*
  The Mutex struct provides a basic lock primative that processes can use for
  synchronization.
*/

struct Mutex {
	volatile int locked;
  ProcessDescriptor *current_owner;
	WaitQueue waitqueue;
};




/*
  The CondVar struct provides a primitive that we can use to impliment condition
  variables.
*/

struct CondVar {
    WaitQueue waitqueue;
};





/* =============================== *

  		      Macros

 * =============================== */

/*
  Macros and functions to create an initialize waitqueues
*/

// Statically initialize a new waitqueue
#define waitQueue(name) { linkedListNode((name).head) }

// Dynamically initialize a new waitqueue
#define waitQueueInit(name) \
    linkedListNodeInit(&(name)->head)

// Create a new waitqueue variable
#define newWaitQueue(name) \
    WaitQueue name = waitQueue(name)



// Statically initialize a new waitqueue node
#define waitQueueNode(name) { 1, &wakeUpProcess, getCurrentProcess(), linkedListNode((name).node) }

// Dynamically initialize a new waitqueue node
#define waitQueueNodeInit(name) \
    (name)->is_exclusive = 0; \
    (name)->prepareToWakeUp = &wakeUpProcess; \
    (name)->process = getCurrentProcess(); \
    linkedListNodeInit(&(name)->node)

// Create a new waitqueue node variable
#define newWaitQueueNode(name) \
    WaitQueueNode name = waitQueueNode(name)




/*
  Macros and functions to create an initialize mutexes
*/

// Statically initialize a mutex
#define mutex(name) { 0, 0, waitQueue((name).waitqueue) }

// Dynamically initialize a mutex
#define mutexInit(name) \
	(name)->locked = 0; \
  (name)->current_owner = 0; \
	waitQueueInit(&(name)->waitqueue)

// Create a new mutex variable
#define newMutex(name) \
	Mutex name = mutex(name)




/*
  Macros and functions to create an initialize condition variables
*/

// Statically initialize a condition variable
#define cvar(name) { waitQueue((name).waitqueue) }

// Dynamically initialize a condition variable
#define cvarInit(name) \
    waitQueueInit(&(name)->waitqueue)

// Create a new mutex variable
#define newCvar(name) \
    CondVar name = condVar(name)





/* =============================== *

             Interface

 * =============================== */

void aquireSpinlock(Spinlock *lock);
void releaseSpinlock(Spinlock *lock);

void addToWaitQueue(WaitQueueNode *node, WaitQueue *head);
int sleepOnWaitQueue(WaitQueue *head);
int sleepOnWaitQueueWithOptions(WaitQueue *head, int exclusive);
void signalWaitQueue(WaitQueue *head);
void signalWaitQueueWithOptions(WaitQueue *head, int exclusive);

void putProcessToSleep();
int wakeUpProcess(WaitQueueNode *node);


Resource* createResourceWithType(enum ResourceType type);
Resource* getResourceWithID(int id, enum ResourceType type);


int mutexInitialize(int *mutex_id);
int mutexAcquire(int mutex_id);
int mutexRelease(int mutex_id);


int cvarInitialize(int *cvar_id);
int cvarWait(int cvar_id, int mutex_id);
int cvarSignal(int cvar_id);
int cvarBroadcast(int cvar_id);



#endif
