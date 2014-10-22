/*
  File: test.c
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

int main() {

	while (1) {
		TracePrintf(1, "Running Test... PID = %d\n", GetPid());
		Pause();
	}
	return 0;
}
