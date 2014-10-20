/*
  File: init_memory.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  	         Includes

 * =============================== */

#include <stdlib.h>
#include <string.h>

#include "../include/hardware.h"
#include "../process/process.h"
#include "../core/list.h"
#include "../memory/memory.h"




/* =============================== *

  	           Data

 * =============================== */

int VIRTUAL_MEMORY_ENABLED = 0;
void *KERNEL_DATA = 0;





/* =============================== *

  	       Implementation

 * =============================== */

/*
  Create a linked list of available page frames, where each node is stored at the
  bottom of a page frame in physical memory. This function can only be run before
  virtual memory is enabled.
*/

void initFrameList() {
	TracePrintf(2, "Initializing frame list\n");

	for (long i = indexOfPage(KERNEL_BRK); i < indexOfPage(PMEM_SIZE); i++) {
		LinkedListNode *node = (LinkedListNode *) (PMEM_BASE + (long)pageAtIndex(i));
		if (i >= indexOfPage(KERNEL_STACK_BASE) && i < indexOfPage(KERNEL_STACK_LIMIT)) continue;

		// If this node is right before or right after the stack, we have to do some special trickery
		if (i == indexOfPage(KERNEL_STACK_BASE) - 1) {
			node->prev = (LinkedListNode *) pageAtIndex(i-1);
			node->next = (KERNEL_STACK_LIMIT < PMEM_SIZE ? (LinkedListNode *) KERNEL_STACK_LIMIT : &frame_head);
		}
		else if (i == indexOfPage(KERNEL_STACK_LIMIT)) {
			node->prev = (LinkedListNode *) (KERNEL_STACK_BASE - PAGESIZE);
			node->next = (i == indexOfPage(PMEM_SIZE)-1 ? &frame_head : (LinkedListNode *) pageAtIndex(i+1));
		}

		// Otherwise, initialize the node normally
		else {
			node->prev = (i == indexOfPage(KERNEL_BRK) ? &frame_head : (LinkedListNode *) pageAtIndex(i-1));
			node->next = (i == indexOfPage(PMEM_SIZE)-1 ? &frame_head : (LinkedListNode *) pageAtIndex(i+1));
		}
	}

	frame_head.prev = (LinkedListNode *) (PMEM_SIZE - PAGESIZE);
	frame_head.next = (LinkedListNode *) KERNEL_BRK;
}




/*
  Initialize the page table for REGION 0. This will set up the virtual address space
  in REGION 0 as follows:

  Kernel TEXT and DATA:  low memory
  Kernel HEAP and STACK: high memory

  Note: $data_start and $heap_start should contain physical addresses, since
  virtual memory won't have been initialized yet.
*/

void initKernelPageTable() {
	TracePrintf(2, "Initializing kernel page table\n");

	long text_options = PTE_VALID | PTE_PERM_READ | PTE_PERM_EXEC;
	long data_options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;

	clearPageTable(&kernel_page_table);

	// First, map the physical pages containing the kernel text and data to the same addresses
	// in the virtual address space
	for (long i=0; i<indexOfPage(KERNEL_DATA); i++) {
		kernel_page_table.entries[i] = createPTEWithOptions(text_options, i);
		frc_table[i] = 1;
	}
	for (long i=indexOfPage(KERNEL_DATA); i<indexOfPage(KERNEL_BRK); i++) {
		kernel_page_table.entries[i] = createPTEWithOptions(data_options, i);
		frc_table[i] = 1;
	}

	// Allocate some page frames for the stack
	for (long i=indexOfPage(KERNEL_STACK_BASE); i<indexOfPage(KERNEL_STACK_LIMIT); i++) {
		kernel_page_table.entries[i] = createPTEWithOptions(data_options, i);
		frc_table[i] = 1;
	}
}




/*
  Initialize the Frame Reference Count table
*/

void initFRCTable() {
	TracePrintf(2, "Initializing FRC table\n");

	frc_table = (char *) malloc(indexOfPage(PMEM_SIZE) * sizeof(char));
	if (frc_table == NULL) {
		TracePrintf(1, "There's no space available for the FRC table'!\n");
		Halt();
	}

	memset(frc_table, 0x00, indexOfPage(PMEM_SIZE) * sizeof(char));
}
