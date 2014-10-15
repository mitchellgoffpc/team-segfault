/*
  File: mutex.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  			  Includes

 * =============================== */

#include "mutex.h"



/* =============================== *

  		   Implementation

 * =============================== */

int mutexAcquire(Mutex *mutex) {

	ProcessDescriptor *me = getCurrentProcess();
	sleepOnWaitQueue(me, &mutex->waitqueue);
	
}