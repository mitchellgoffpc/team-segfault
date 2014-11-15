/*
  File: threads.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  			  Includes

 * =============================== */

#include "../include/hardware.h"
#include "../include/yalnix.h"

#include "threads.h"





/* =============================== *

  		   Implementation

 * =============================== */

/*
  Spawn a new thread
*/

int CreateThread(ThreadEntry entry, void *entry_arg) {
	int thread_id = Custom0(0, 0, 0, 0);
	if (thread_id == ERROR) return ERROR;

	// If we're the child, go to the entry point
	if (thread_id == 0) {
		entry(entry_arg);
		Exit(0);
	}

	// If we're the parent, return with the child's thread id
	else {
		return thread_id;
	}
}



/*
  Wait for one of our child threads to finish before continuning
*/

int JoinThread(int thread_id) {
	return Custom1(thread_id, 0, 0, 0);
}
