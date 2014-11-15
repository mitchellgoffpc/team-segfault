/*
  File: test.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

             Includes

 * =============================== */

#include <stdlib.h>
#include <string.h>

#include "../include/hardware.h"
#include "../include/yalnix.h"

#include "threads.h"
#include "threads.c"





/* =============================== *

             Functions

 * =============================== */

int flipper = 0;
int cvar_id, lock_id;

void testThread(void *pid) {
	TracePrintf(1, "I'm a thread!\n");

	// while (1) {
	// 	Acquire(lock_id);
	// 	TracePrintf(1, "Acquired lock! Parent: %d\n", pid > 0);

	// 	while ((pid > 0 && flipper > 0) || (pid == 0 && flipper == 0)) {
	// 		CvarWait(cvar_id, lock_id);
	// 	}

	// 	Delay(3);
	// 	TracePrintf(1, "Flipping value from %d to %d! Parent: %d\n", pid > 0, pid == 0, pid > 0);
	// 	flipper = pid ? 1 : 0;
	// 	CvarSignal(cvar_id);

	// 	TracePrintf(1, "Released lock! Parent: %d\n", pid > 0);
	// 	Release(lock_id);
	// }
}


int main() {
	CvarInit(&cvar_id);
	LockInit(&lock_id);
	
	int thread_id = CreateThread(testThread, 0);
	// testThread((void *) 1);
	JoinThread(thread_id);

	return 0;
}



	// while (1) {
	// 	TracePrintf(1, "Running Test... PID = %d\n", GetPid());
	// 	Pause();
	// }

	// int pid = Fork();
	// if (pid == 0) {
	// 	Delay(5);
	// 	TracePrintf(1, "Child exiting\n");
	// 	return 0;
	// }

	// int status;
	// Wait(&status);

	// pid = Fork();
	// if (pid == 0) {
	// 	int zero = 0;
	// 	pid = 5/zero;
	// 	return 0;
	// }




	// char buffer[2056];
	// for (int i=0; i<2056; i++) { buffer[i] = '1'; }
	// buffer[2055] = '\n';
	// TtyWrite(0, &buffer, 2056);

	// char prompt[] = "Type something in:\n";
	// TtyWrite(0, &prompt, sizeof(prompt));

	// long length = TtyRead(0, &buffer, 2056);
	// buffer[length] = 0;
	// TracePrintf(1, "Input: %s\n", buffer);

	
	// TracePrintf(1, "Parent exiting\n");



	// int lock_id;
	// LockInit(&lock_id);

	// int pid = Fork();

	// Acquire(lock_id);
	// TracePrintf(1, "Acquired lock!\n");
	
	// Delay(5);

	// TracePrintf(1, "Released lock!\n");
	// Release(lock_id);
