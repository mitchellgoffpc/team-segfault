/*
  File: mutex.h
  Date: 10/8/2014
  Author: Mitchell Goff
*/

#ifndef __YALNIX_MUTEX_H__
#define __YALNIX_MUTEX_H__



/* =============================== *

  			  Includes

 * =============================== */

#include "waitqueue.h"




/* =============================== *

  		   Data Structures

 * =============================== */

struct Mutex;

typedef struct Mutex Mutex;




/*
  The Mutex struct provides a basic lock primative that processes can use for
  synchronization.
*/

struct Mutex {
	Spinlock waitqueue_lock;
	volatile int lock;
	WaitQueue waitqueue;
}




/* =============================== *

  		      Macros

 * =============================== */

#define mutex(name) { 0, waitQueue(name.waitqueue) }

#define mutexInit(name) \
	name->lock = 0; \
	waitQueueInit(&name->waitqueue)

#define newMutex(name) \
	Mutex name = mutex(name)



#endif
