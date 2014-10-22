/*
  File: idle.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

             Includes

 * =============================== */

#include "../include/hardware.h"
#include "../include/yalnix.h"





/* =============================== *

             Functions

 * =============================== */

int test = 32;

int main() {
	
	// int pid = Fork();
	// TracePrintf(1, "Returned from fork! Child's PID: %d\n", pid);

	if (Fork() == 0) {
		char *args = NULL;
		Exec("apps/test", &args);
		// test = 48;
	}

	// if (Fork() == 0) {
	// 	test = 64;
	// }

	while (1) {
		TracePrintf(1, "Running Idle... PID = %d\n", GetPid());
		// TracePrintf(1, "test = %d\n", test);
		Pause();
	}
	return 0;
}
