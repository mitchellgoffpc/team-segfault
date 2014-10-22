/*
  File: traps.c
  Date: 10/6/2014
  Author: Emily Holt & Mitchell Goff
*/



/* =============================== *

              Includes

 * =============================== */

#include "../include/hardware.h"
#include "../include/yalnix.h"

#include "../memory/memory.h"
#include "../process/process.h"
#include "traps.h"





/* =============================== *

             Interrupts

 * =============================== */

/*
  When a TRAP_CLOCK interrupt is received, call schedule to switch to the next process
*/

void trapClock(UserContext *context) {
    TracePrintf(1, "TRAP_CLOCK\n");

    saveUserContext();
    schedule();
    restoreUserContext();
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
    // TracePrintf(1, "TRAP_KERNEL\n");
    saveUserContext();
    int result;

    switch(context->code) {
        case YALNIX_FORK:
            result = forkProcess();
            getCurrentProcess()->user_context.regs[0] = result;
            break;
        
        case YALNIX_EXEC:
            result = loadProgram(
                (char *) getCurrentProcess()->user_context.regs[0],
                (char **) getCurrentProcess()->user_context.regs[1]);
            getCurrentProcess()->user_context.regs[0] = result;
            break;

        case YALNIX_GETPID:
            getCurrentProcess()->user_context.regs[0] = getCurrentProcess()->pid;
            break;
    }

    restoreUserContext();
}





/* =============================== *

             Exceptions

 * =============================== */

/*
  When a TRAP_ILLEGAL interrupt is recieved, kill the current process
*/

void trapIllegal(UserContext *context) {
    TracePrintf(1, "TRAP_ILLEGAL\n");
    Halt();
    // kill the current process
    // TracePrintf(message: process ID and explanation of problem)
}




/*
  When a TRAP_MATH interrupt is recieved, kill the current process
*/

void trapMath(UserContext *context) {
    TracePrintf(1, "TRAP_MATH\n");
    Halt();
    // kill the process
    // TracePrintf(message: process ID and explanation of problem)
}




/*
  When a TRAP_MEMORY interrupt is recieved, do some sanity checking and grow the
  user's heap/stack
*/

void trapMemory(UserContext *context) {
    // TracePrintf(1, "TRAP_MEMORY\n");
    saveUserContext();
    
    void *address = getCurrentProcess()->user_context.addr;
    handleMemoryTrap(address);

    restoreUserContext();
}
