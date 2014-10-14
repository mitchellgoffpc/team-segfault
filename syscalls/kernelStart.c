//KernelStart
//Authors: Emily Holt and Mitchell Goff
#include <stdio.h>

//headerfiles to include
#include "hardware.h"

//files that include the trap functions
#include "trapClock.c"
#include "trapIllegal.c"
#include "trapKernel.c"
#include "trapMath.c"
#include "trapMemory.c"
#include "trapTtyReceive.c"
#include "trapTtyTransmit.c"


//the interrupt vector table
int (*interrupt_vector[TRAP_VECTOR_SIZE]);
int first_free_index = 8;

//function prototypes for traps
void trapClock(UserContext *);
void trapIllegal(UserContext *);
void trapKernel(UserContext *);
void trapMath(UserContext *);
void trapMemory(UserContext *);
void trapTtyReceive(UserContext *);
void trapTtyTransmit(UserContext *);

//function prototypes for kernelStart
void initializeInterruptVector();

void kernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt) {
	
	//initialize interrupt vector tables 
	//by making them point to the correct handler functions
	initializeInterruptVector();

	//initialize the REG_VECTOR_BASE privileged machine register 
	//to point to interrupt vector table

	//stuct to keep track of what page frames in physical memory are free
	//list of free pages should be based on pmem_size

	//build initial page tables for Region0 and Region1

	//enable virtual memory

	//create an idle process that executes the Pause machine instruction 

	//create the first process and load initial program into it

	//construct contexts of init and idle process
	//save these contexts so that the contexts can be switched in and out
	//write context of init process into currently active context (uctxt)

	//return

}

void initializeInterruptVector() {

	//initialize vector entries to point to functions
	interrupt_vector[0] = trapKernel;
	interrupt_vector[1] = trapClock;
	interrupt_vector[2] = trapIllegal;
	interrupt_vector[3] = trapMemory;
	interrupt_vector[4] = trapMath;
	interrupt_vector[5] = trapTtyReceive;
	interrupt_vector[6] = trapTtyTransmit;
	interrupt_vector[7] = trapDisk;

	//initialize the rest of the entries to point to null
	int i;
	i = first_free_index;
	while (i < TRAP_VECTOR_SIZE) {
		interrupt_vector[i] = NULL;
	}

	return 0

}