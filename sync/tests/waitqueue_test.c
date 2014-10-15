/* Tests for waitqueue.h */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../waitqueue.h"
#include "../../process/process.h"


void testWaitQueue() {
	ProcessDescriptor process_a;
	ProcessDescriptor process_b;

	newWaitQueue(waitqueue_a);

	sleepOnWaitQueue(&process_a, &waitqueue_a);
	assert(process_a.state == PROCESS_WAITING);

	sleepOnWaitQueueWithOptions(&process_b, &waitqueue_a, 1);
	assert(process_b.state == PROCESS_WAITING);
	
	signalWaitQueue(&waitqueue_a);
	assert(process_a.state == PROCESS_RUNNING);
	assert(process_b.state == PROCESS_RUNNING);
}



int main() {
	testWaitQueue();

	printf("All tests passed!\n");
	return 0;
}