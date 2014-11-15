/*
  File: cvar.c
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
  Create a new condition variable and add it to the list of resources
*/

int cvarInitialize(int *cvar_id) {
	Resource *resource = createResourceWithType(RESOURCE_CVAR);
	errorIfNull(resource, "Couldn't allocate enough space for a new resource\n");

	resource->location = malloc(sizeof(CondVar));
	errorIfNull(resource->location, "Couldn't allocate enough space for a new condition variable\n");

	cvarInit((CondVar *) resource->location);
	*cvar_id = resource->id;
}




/*
  Release the mutex, wait to be signaled, then acquire the mutex again before returning
*/

int cvarWait(int cvar_id, int mutex_id) {
	Resource *resource = getResourceWithID(cvar_id, RESOURCE_CVAR);
	errorIfNull(resource, "It looks like you passed in a non-valid cvar id\n");
	CondVar *cvar = (CondVar *) resource->location;

	checkForError (mutexRelease(mutex_id));
	checkForError (sleepOnWaitQueue(&cvar->waitqueue));
	checkForError (mutexAcquire(mutex_id));
}




/*
  Signal a single process on the condition variable's waitqueue
*/

int cvarSignal(int cvar_id) {
	Resource *resource = getResourceWithID(cvar_id, RESOURCE_CVAR);
	errorIfNull(resource, "It looks like you passed in a non-valid cvar id\n");
	CondVar *cvar = (CondVar *) resource->location;

	signalWaitQueue(&cvar->waitqueue);
}




/*
  Signal all the processes on the condition variable's waitqueue
*/

int cvarBroadcast(int cvar_id) {
	Resource *resource = getResourceWithID(cvar_id, RESOURCE_CVAR);
	errorIfNull(resource, "It looks like you passed in a non-valid cvar id\n");
	CondVar *cvar = (CondVar *) resource->location;

	signalWaitQueueWithOptions(&cvar->waitqueue, 0);
}
