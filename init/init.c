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

// A dummy idle program for testing
void DoIdle() {
	TracePrintf(1, "DoIdle\n");
	while(1) { }
}




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

void loadInit() {
	TracePrintf(1, "Loading the init process...\n");

	PageTable *table = (PageTable *) malloc(sizeof(PageTable));
	if (!table) {
    	TracePrintf(1, "Couldn't find enough space for a new process descriptor!\n");
    	Halt();
    }
	clearPageTable(table);

	ProcessDescriptor *process = (ProcessDescriptor *) malloc(sizeof(ProcessDescriptor));
    if (!process) {
    	TracePrintf(1, "Couldn't find enough space for a new process descriptor!\n");
    	Halt();
    }

    processDescriptorInit(process);
    if (process->pid == 0) {
    	TracePrintf(1, "No available PIDs!\n");
    	Halt();
    }

    for (int i=0; i<indexOfPage(KERNEL_STACK_MAXSIZE); i++) {
    	process->pcb_frames[i] = (void *) (KERNEL_STACK_BASE + (long)pageAtIndex(i));
    }

    process->page_table = table;
    WriteRegister(REG_PTBR1, (long)table);
	WriteRegister(REG_PTLR1, VMEM_REGION_SIZE >> PAGESHIFT);

	DoIdle();
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

<<<<<<< HEAD:init/init.c
=======
	// For now, just create an idle process that executes the Pause machine instruction 
	Pause();
	

	initIdle();
	

	//need to load DoIdle into UserText

	context.pc = VMEM_1_BASE + (DoIdle & PAGEOFFSET);

}


int initIdle() {
	PageTable *table = (PageTable*) malloc(sizeof(PageTable));

	long text_options = PTE_VALID | PTE_PERM_READ | PTE_PERM_EXEC;
	long data_options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;

	PTE entry = kernel_page_table.entries[(long)DoIdle >> PAGESHIFT];
	PTE new_entry = createPTEWithOptions(text_options, (long)entry.pfn);
	long offset = DoIdle & PAGESHIFT;

	kernel_page_table.entries[i] = createPTEWithOptions(text_options, offset);
	
	for (long i=indexOfPage(KERNEL_BRK); i<indexOfPage(VMEM_REGION_SIZE); i++) {
		table->entries[i] = createPTEWithOptions(0, 0);
	}

	for (long i=indexOfPage(KERNEL_STACK_BASE); i<indexOfPage(KERNEL_STACK_LIMIT); i++) {
		table->entries[i] = createPTEWithOptions(data_options, i);
	}
}

/*
  Initialize the interrupt vector
*/

void initializeInterruptVector(void) {

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
	for (int i=first_free_index; i<TRAP_VECTOR_SIZE; i++) {
		interrupt_vector[i] = NULL;
	}
}
>>>>>>> FETCH_HEAD:core/init.c

	// Start the init process
	loadInit();
}