/*
  File: process.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  			 Includes

 * =============================== */

#include <stdlib.h>
#include <string.h>

#include "../include/hardware.h"
#include "../memory/memory.h"
#include "../core/list.h"
#include "process.h"





/* =============================== *

  		  Data Structures

 * =============================== */

LinkedListNode process_head = linkedListNode(process_head);
long max_pid = 0;




/* =============================== *

  		  Implementation

 * =============================== */

/*
  Create a new process descriptor and PCB
*/

 ProcessDescriptor* createProcessDescriptor() {
 	TracePrintf(2, "Creating a new process descriptor...\n");

	// Try to allocate space for a new process descriptor
    ProcessDescriptor *process = (ProcessDescriptor *) malloc(sizeof(ProcessDescriptor));
    if (!process) return 0;

    // Initialize it and check if there are any available PIDs left
    processDescriptorInit(process);
    if (process->pid == 0) { free(process); return 0; }

    // Try to allocate space for a new process control block
    for (int i=0; i<indexOfPage(KERNEL_STACK_MAXSIZE); i++) {
        process->pcb_frames[i] = allocatePageFrame();
        
        // If there wasn't enough room, free any page frames we've already allocated
        if (!process->pcb_frames[i]) {
        	for (int j=i; j>=0; j--) { freePageFrame(process->pcb_frames[j]); }
        	free(process);
        	return 0;
        }
    }

    // Create the ProcessInfo struct at the bottom of the stack
    long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
    frame_window_pte(0) = createPTEWithOptions(options, indexOfPage(process->pcb_frames[0]));
    ((ProcessInfo *) frame_window(0))->descriptor = process;

    TracePrintf(2, "Successfully created process descriptor!\n");
    return process;
}




/*
  Something else
*/

