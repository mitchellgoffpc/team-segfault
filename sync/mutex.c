/*
  File: mutex.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  			  Includes

 * =============================== */

#include <stdlib.h>

#include "../process/process.h"
#include "sync.h"





/* =============================== *

  		   Implementation

 * =============================== */

/*
  Create a new mutex and add it to the list of resources
*/

int mutexInitialize(int *mutex_id) {
	Resource *resource = createResourceWithType(RESOURCE_MUTEX);
	errorIfNull(resource, "Couldn't allocate enough space for a new resource\n");

	resource->location = malloc(sizeof(Mutex));
	errorIfNull(resource->location, "Couldn't allocate enough space for a new mutex\n");

	mutexInit((Mutex *) resource->location);
	*mutex_id = resource->id;
}




/*
  Sleep on the mutex's waitqueue until the mutex becomes unlocked
*/

int mutexAcquire(int mutex_id) {
	Resource *resource = getResourceWithID(mutex_id, RESOURCE_MUTEX);
	errorIfNull(resource, "It looks like you passed in a non-valid mutex id\n");
	Mutex *mutex = (Mutex *) resource->location;

	// If we've already acquired this mutex, return immediately
	if (mutex->current_owner == getCurrentProcess()) return 0;

	// Wait until the mutex becomes available
	while (mutex->locked) {
		checkForError(sleepOnWaitQueue(&mutex->waitqueue));
	}

	// Lock the mutex
	mutex->locked = 1;
	mutex->current_owner = getCurrentProcess();
}




/*
  Wake up the next process on the mutex's waitqueue
*/

int mutexRelease(int mutex_id) {
	Resource *resource = getResourceWithID(mutex_id, RESOURCE_MUTEX);
	errorIfNull(resource, "It looks like you passed in a non-valid mutex id\n");
	Mutex *mutex = (Mutex *) resource->location;

	mutex->locked = 0;
	mutex->current_owner = 0;
	signalWaitQueue(&mutex->waitqueue);
}
