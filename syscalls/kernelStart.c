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


typedef void (*InterruptHandler) (UserContext*);

//the interrupt vector table
InterruptHandler interrupt_vector[TRAP_VECTOR_SIZE];

int first_free_index = 8;

//function prototypes for traps
void trapClock(UserContext *);
void trapIllegal(UserContext *);
void trapKernel(UserContext *);
void trapMath(UserContext *);
void trapMemory(UserContext *);
void trapTtyReceive(UserContext *);
void trapTtyTransmit(UserContext *);
void trapDisk(UserContext *);

//function prototypes for kernelStart
void initializeInterruptVector(void);
void DoIdle(void);

void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt) {
	
	//initialize interrupt vector tables 
	//by making them point to the correct handler functions
	initializeInterruptVector();

	//initialize the REG_VECTOR_BASE privileged machine register 
	//to point to interrupt vector table
	WriteRegister(REG_VECTOR_BASE, (unsigned int) interrupt_vector);

	//stuct to keep track of what page frames in physical memory are free
	//list of free pages should be based on pmem_size

	//build initial page tables for Region0 and Region1

	//enable virtual memory (from p.28)
	WriteRegister(REG_VM_ENABLE, 1);

	//create an idle process that executes the Pause machine instruction 
	//DoIdle();

	//create the first process and load initial program into it by
	//setting up the stack then the
	//program counter needs to point to DoIdle
	PageTable *table = (PageTable*) malloc(sizeof(PageTable));

	uctxt.pc = DoIdle; //address of DoIdle;


	/* ***For future checkpoints*** */
	//construct contexts of init and idle process
	//save these contexts so that the contexts can be switched in and out
	//write context of init process into currently active context (uctxt)


	//return
	return;

}

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

	//initialize the rest of the entries to point to null
	int i;
	i = first_free_index;
	while (i < TRAP_VECTOR_SIZE) {
		interrupt_vector[i] = NULL;
		i++;
	}

	return;

}

void DoIdle(void) {
	while(1) {
		TracePrintf(1, "DoIdle\n");
		Pause();
	}
}

