/*
  File: traps.c
  Date: 10/6/2014
  Author: Emily Holt & Mitchell Goff
*/



/* =============================== *

  	          Includes

 * =============================== */

#include "../include/hardware.h"
#include "traps.h"




/* =============================== *

  	         Interrupts

 * =============================== */

/*
  When a TRAP_CLOCK interrupt is received, call schedule to switch to the next process
*/

void trapClock(UserContext *context) {
  TracePrintf(1, "TRAP_CLOCK\n");
	// schedule();
}




/*
  When a TRAP_DISK interrupt is recieved, do something with the information
*/

void trapDisk(UserContext *context) {
  TracePrintf(1, "TRAP_DISK\n");
	// pass
}




/*
  When a TRAP_TTY_RECEIVE interrupt is recieved, do something with the information
*/

void trapTtyReceive(UserContext *context) {
  TracePrintf(1, "TRAP_TTY_RECEIVE\n");
	// pass
}




/*
  When a TRAP_TTY_TRANSMIT interrupt is recieved, do something with the information
*/

void trapTtyTransmit(UserContext *context) {
  TracePrintf(1, "TRAP_TTY_TRANSMIT\n");
	// pass
}





/* =============================== *

  	           Traps

 * =============================== */

/*
  When a TRAP_KERNEL interrupt is recieved, trigger the appropriate syscall
*/

void trapKernel(UserContext *context) {
  TracePrintf(1, "TRAP_KERNEL\n");
	int call_number = context->regs[0];

	// load the requested kernel call
	// execute the requested kernel call
	// save the return value from the kernel call in regs[0]

	// UserContext.regs[0] = return val
}





/* =============================== *

  	         Exceptions

 * =============================== */

/*
  When a TRAP_ILLEGAL interrupt is recieved, kill the current process
*/

void trapIllegal(UserContext *context) {
  TracePrintf(1, "TRAP_ILLEGAL\n");
	// kill the current process
	// TracePrintf(message: process ID and explanation of problem)
}




/*
  When a TRAP_MATH interrupt is recieved, kill the current process
*/

void trapMath(UserContext *context) {
  TracePrintf(1, "TRAP_MATH\n");
  // kill the process
  // TracePrintf(message: process ID and explanation of problem)
}




/*
  When a TRAP_MEMORY interrupt is recieved, do some sanity checking and grow the
  user's heap/stack
*/

void trapMemory(UserContext *context) {
  TracePrintf(1, "TRAP_MEMORY\n");
	void *addr = context->addr;
}
