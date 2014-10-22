/*
  File: init.c
  Date: 10/6/2014
  Author: Emily Holt & Mitchell Goff
*/



/* =============================== *

  	         Includes

 * =============================== */

#include <stdlib.h>
#include <string.h>

#include "../include/hardware.h"
#include "../memory/memory.h"
#include "../traps/traps.h"
#include "../process/process.h"




/* =============================== *

  	      Data Structures

 * =============================== */

#define NUMBER_OF_TRAP_HANDLERS 8

typedef void (*InterruptHandler) (UserContext *);
InterruptHandler interrupt_vector[TRAP_VECTOR_SIZE];




/* =============================== *

  	    Helper Functions

 * =============================== */

<<<<<<< HEAD
=======

// A dummy idle program for testing
void DoIdle() {
	
	while(1) { 
		TracePrintf(1, "DoIdle\n");
		Pause();
	}
}

/*	
Method: delay the calling process for the given number of clock ticks
*/

void KernelDelay(UserContext *context) {
	int clock_ticks;
	clock_ticks = context->regs[0];

	//if clock_ticks is 0 return immediately
	if (clock_ticks == 0) return 0;

	//if clock_ticks < 0 don't time travel, return an Error
	if (clock_ticks < 0) {
		TracePrintf(1, "Hey! We can't go back in time!\n");
		return -1;
	}

	//block the calling process until clock_ticks clock interrupts have occurred
	int counter;
	counter = 0;
	while (counter < clock_ticks) {
		//put the idle process on the ready queue
		//call a trapClock to kill the switch to
		//the next process on the ready queue
		//which will be doIdle 
		counter++;
	}
	

}



>>>>>>> FETCH_HEAD
/*
  Initialize the interrupt vector
*/

void initializeInterruptVector() {

	//initialize vector entries to point to functions
	interrupt_vector[TRAP_KERNEL] = trapKernel;
	interrupt_vector[TRAP_CLOCK] = trapClock;
	interrupt_vector[TRAP_ILLEGAL] = trapIllegal;
	interrupt_vector[TRAP_MEMORY] = trapMemory;
	interrupt_vector[TRAP_MATH] = trapMath;
	interrupt_vector[TRAP_TTY_RECEIVE] = trapTtyReceive;
	interrupt_vector[TRAP_TTY_TRANSMIT] = trapTtyTransmit;
	interrupt_vector[TRAP_DISK] = trapDisk;

	// Set the rest of the entries to null
	for (int i=NUMBER_OF_TRAP_HANDLERS; i<TRAP_VECTOR_SIZE; i++) {
		interrupt_vector[i] = NULL;
	}
}




/*
  Load the init process into memory and start running it
*/

void loadInit(UserContext *context) {
	TracePrintf(1, "Getting read to load 'idle'...\n");

	// Try to allocate space for the REGION_1 page table
	PageTable *table = (PageTable *) malloc(sizeof(PageTable));
	haltIfNull(table, "There's not enough space for a new page table!\n");
    clearPageTable(table);

	// Try to allocate space for the process descriptor
	ProcessDescriptor *process = (ProcessDescriptor *) malloc(sizeof(ProcessDescriptor));
    haltIfNull(table, "There's not enough space for a new process descriptor!\n");
    processDescriptorInit(process);

    // Keep a record of which page frames the new kernel stack was allocated in
    for (int i=0; i<indexOfPage(KERNEL_STACK_MAXSIZE); i++) {
    	process->pcb_frames[i] = (void *) (KERNEL_STACK_BASE + (long)pageAtIndex(i));
    }


    // Update the process descriptor and tell the machine where the REGION_1
    // page table is located
    process->page_table = table;
    memcpy(&process->user_context, context, sizeof(UserContext));

    ((ProcessInfo *) KERNEL_STACK_BASE)->descriptor = process;
    addLastNode(&process->process_list, &process_head);

    WriteRegister(REG_PTBR1, (long)table);
	WriteRegister(REG_PTLR1, VMEM_REGION_SIZE >> PAGESHIFT);


	// Load init into memory and start running it
	char *args = NULL;
	if (loadProgram("apps/idle", &args) < 0) {
		TracePrintf(0, "Idle failed to load :(\n");
		Halt();
	}

	memcpy(context, &process->user_context, sizeof(UserContext));
}





/* =============================== *

  	      Core Functions

 * =============================== */

/*
  Catch some basic parameters about position of the kernel in memory
*/

void SetKernelData(void *kernel_data_start, void *kernel_data_end) {
	TracePrintf(1, "Called SetKernelData\n");

	KERNEL_DATA = kernel_data_start;
	KERNEL_BRK = (void *) UP_TO_PAGE(kernel_data_end);
}




/*
  Initialize the page table and interrupt vector and start running idle
*/

void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *context) {
	TracePrintf(1, "Called KernelStart\n");
	
	// Build the interrupt vector and set the REG_VECTOR_BASE register to point to it
	WriteRegister(REG_VECTOR_BASE, (long)interrupt_vector);
	initializeInterruptVector();

	// Initialize the page table
	TracePrintf(2, "Creating Page Tables\n");
	PMEM_SIZE = pmem_size;
	
	initFRCTable();
	initFrameList();
	initKernelPageTable();


	// Enable virtual memory
	TracePrintf(1, "Enabling virtual memory...\n");
	TracePrintf(2, "Page Table Address: %lX\n", (long)&kernel_page_table);
	TracePrintf(2, "Kernel Brk: %lX\n", KERNEL_BRK);
	for (int i=0; i<VMEM_REGION_SIZE >> PAGESHIFT; i++) {
		TracePrintf(3, "PTE %d: %lX\n", i, (long)kernel_page_table.entries[i].pfn);
	}


	WriteRegister(REG_PTBR0, (long)&kernel_page_table);
	WriteRegister(REG_PTLR0, VMEM_REGION_SIZE >> PAGESHIFT);

	WriteRegister(REG_VM_ENABLE, 1);
	VIRTUAL_MEMORY_ENABLED = 1;

	//start our clock
	ellapsed_clock_ticks = 0;

	// Start the init process
	loadInit(context);
}
