/*
  File: tty.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

             Includes

 * =============================== */

#include <stdlib.h>

#include "../include/hardware.h"
#include "../include/yalnix.h"

#include "../memory/memory.h"
#include "../process/process.h"
#include "../sync/sync.h"
#include "traps.h"





/* =============================== *

               Data

 * =============================== */

TTY ttys[NUM_TERMINALS];





/* =============================== *

             Interface

 * =============================== */

/*
  Copy the contents of the tty read buffer into the user space
*/

int ttyRead(int tty, void *u_buffer, int u_length) {
	if (tty >= NUM_TERMINALS || tty < 0) return ERROR;

	// First, check if there's any data ready right now
	WaitQueue *queue = &ttys[tty].read_queue;
	while (!ttys[tty].read_buffer || ttys[tty].read_buffer_position >= ttys[tty].read_buffer_size) {
		sleepOnWaitQueue(queue);
	}

	long length = ttys[tty].read_buffer_size;
	long position = ttys[tty].read_buffer_position;

	// Then send a chunk of the buffer to the user
	int sub_length = length - position > u_length ? u_length : length - position;
	void *buffer = (void *) ((long)ttys[tty].read_buffer + position);
	memcpy(u_buffer, buffer, sub_length);
	ttys[tty].read_buffer_position += sub_length;

	signalWaitQueue(&ttys[tty].read_queue);
	return sub_length;
}




/*
  Copy some data from a terminal into the kernel space
*/

void ttyReadBegin(int tty) {
	if (tty >= NUM_TERMINALS || tty < 0) return;

	// First, make some space for the data
	free(ttys[tty].read_buffer);
	ttys[tty].read_buffer = malloc(TERMINAL_MAX_LINE);
	if (ttys[tty].read_buffer == NULL) {
		TracePrintf(0, "Cannot read from terminal: Not enough space for buffer\n");
		return;
	}

	// Then copy the data into the buffer
	long length = TtyReceive(tty, ttys[tty].read_buffer, TERMINAL_MAX_LINE);
	TracePrintf(0, "Length: %ld\n", length);
	ttys[tty].read_buffer_size = length;
	ttys[tty].read_buffer_position = 0;

	signalWaitQueue(&ttys[tty].read_queue);
}




/*
  Write the contents of a buffer to the TTY

  Note: We're using some clever manipulation of the non-exclusive option
  for waitqueues. Basically, the process currently writing to the the terminal
  is added to the terminal's waitqueue as a non-exclusive process so it will be
  added to the front of the queue and its output won't be interrupted by another
  process. Then, before signaling the waitqueue, all non-exclusive nodes are
  marked as exclusive so they will act normally.
*/

/*
  Note 2: May change this to a condition variable-based system once they've
  been properly implemented
*/

int ttyWrite(int tty, void *u_buffer, int length) {
	if (tty >= NUM_TERMINALS || tty < 0) return ERROR;

	// First, copy the data into the kernel space
	void *buffer = malloc(length);
	errorIfNull(buffer, "Cannot write to terminal: Not enough space for buffer\n");
	memcpy(buffer, u_buffer, length);
	
	// Then check if there's currently a process waiting for the terminal
	WaitQueue *queue = &ttys[tty].write_queue;
	while (ttys[tty].write_current != 0) {
		sleepOnWaitQueue(queue);
	}
	ttys[tty].write_current = getCurrentProcess()->pid;

	// Then send chunks of the buffer to the terminal until we've sent the whole message
	for (int position = 0; position < length; position += TERMINAL_MAX_LINE) {
		int sub_length = length - position > TERMINAL_MAX_LINE ? TERMINAL_MAX_LINE : length - position;
		TtyTransmit(tty, (void *) ((long)buffer + position), sub_length);

		// Add this process to a waitqueue until the trap returns
		sleepOnWaitQueueWithOptions(queue, 0);
	}

	ttys[tty].write_current = 0;
}




/*
  Unblock the next process that wants to write to the terminal
*/

void ttyWriteFinished(int tty) {
	if (tty >= NUM_TERMINALS || tty < 0) return;
	WaitQueue *queue = &ttys[tty].write_queue;

	// Mark each node as exclusive
	WaitQueueNode *current;
	forEachElement(current, &queue->head, node) {
		if (current->is_exclusive) break;
		current->is_exclusive = 1;
	}

	signalWaitQueue(queue);
}
