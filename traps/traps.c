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
#include "../sync/sync.h"
#include "traps.h"





/* =============================== *

               Data

 * =============================== */

long elapsed_clock_ticks = 0;





/* =============================== *

             Interrupts

 * =============================== */

/*
  When a TRAP_CLOCK interrupt is received, call schedule to switch to the next process
*/

void trapClock(UserContext *context) {
    TracePrintf(1, "TRAP_CLOCK\n");
    saveUserContext();
    
    elapsed_clock_ticks++;
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
    // TracePrintf(1, "TRAP_TTY_RECEIVE\n");
    ttyReadBegin(context->code);
}




/*
  When a TRAP_TTY_TRANSMIT interrupt is recieved, do something with the information
*/

void trapTtyTransmit(UserContext *context) {
    // TracePrintf(1, "TRAP_TTY_TRANSMIT\n");
    ttyWriteFinished(context->code);
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
            register(0) = result;
            break;
        
        case YALNIX_EXEC:
            result = loadProgram((char *) register(0), (char **) register(1));
            register(0) = result;
            break;

        case YALNIX_GETPID: register(0) = getCurrentProcess()->pid; break;
        case YALNIX_DELAY: register(0) = delayProcess(register(0)); break;

        case YALNIX_BRK: register(0) = setProcessBrk((void *) register(0)); break;
        case YALNIX_EXIT: killCurrentProcess(register(0)); break;
        case YALNIX_WAIT: register(0) = waitForPID(1, (int *) register(0)); break;

        
        case YALNIX_TTY_READ:
            register(0) = ttyRead(register(0), (void *) register(1), register(2)); break;
        case YALNIX_TTY_WRITE:
            register(0) = ttyWrite(register(0), (void *) register(1), register(2)); break;


        case YALNIX_LOCK_INIT: register(0) = mutexInitialize((int *) register(0)); break;
        case YALNIX_LOCK_ACQUIRE: register(0) = mutexAcquire(register(0)); break;
        case YALNIX_LOCK_RELEASE: register(0) = mutexRelease(register(0)); break;

        case YALNIX_CVAR_INIT: register(0) = cvarInitialize((int *) register(0)); break;
        case YALNIX_CVAR_WAIT: register(0) = cvarWait(register(0), register(1)); break;
        case YALNIX_CVAR_SIGNAL: register(0) = cvarSignal(register(0)); break;
        case YALNIX_CVAR_BROADCAST: register(0) = cvarBroadcast(register(0)); break;


        case YALNIX_CUSTOM_0:
            result = createThread();
            register(0) = result;
            break;

        case YALNIX_CUSTOM_1: register(0) = joinThread(register(0)); break;
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
    TracePrintf(1, "Process %d has received a TRAP_ILLEGAL\n", getCurrentProcess()->pid);
    killCurrentProcess(-1);
}




/*
  When a TRAP_MATH interrupt is recieved, kill the current process
*/

void trapMath(UserContext *context) {
    TracePrintf(1, "Process %d has received a TRAP_MATH\n", getCurrentProcess()->pid);
    killCurrentProcess(-1);
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
