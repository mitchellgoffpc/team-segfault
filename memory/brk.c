/*
	File: brk.c
	Date: 10/20/14
	Author: Emily Holt and Mitchell Goff
*/

/* =============================== *

  	        Includes

 * =============================== */

#include <string.h>

#include "../core/list.h"
#include "memory.c"





/* =============================== *

  	          Data

 * =============================== */

PageTable user_page_table;





/* =============================== *

  	     Implementation

 * =============================== */

/*
  If we're increasing the size of the heap, allocate some new page frames
  and set the page table entries for the heap to point to them
*/

static int increaseBrk(void *address) {

	// Figure out how many new frames we need to allocate
	long frames_needed = (UP_TO_PAGE(address) - 
		((ProcessInfo *) KERNEL_STACK_BASE)->) >> PAGESHIFT;

	for (int i=0; i<frames_needed; i++) {
		// Allocate a new physical page frame, then create a PTE for it
		void *frame = allocatePageFrame();
		if (frame == NULL) {
			TracePrintf(1, "There aren't any page frames left for the heap :(\n");
			return -1;
		}

		long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
		PTE entry = createPTEWithOptions(options, indexOfPage(frame));

		// Insert the new PTE into the page table
		long index = indexOfPage(((ProcessInfo *) KERNEL_STACK_BASE)->current_brk) + i;
		user_page_table.entries[index] = entry;
	}

	((ProcessInfo *) KERNEL_STACK_BASE)->current_brk = (void *) UP_TO_PAGE(address);
	return 0;
}




/*
  If we're shrinking the heap, free any page frames we no longer need
  and mark the corresponding page table entries as invalid
*/

static int decreaseBrk(void *address) {

	// Figure out how many frames we need to free
	long frames_freed = (((ProcessInfo *) KERNEL_STACK_BASE)->current_brk - 
		UP_TO_PAGE(address)) >> PAGESHIFT;

	for (int i=0; i<frames_freed; i++) {
		// Figure out which physical frame to free
		long pte_index = indexOfPage(UP_TO_PAGE(address)) + i;
		long frame_index = page_table.entries[pte_index].pfn;
		void *frame = pageAtIndex(frame_index);

		// Free the frame and clear the PTE
		freePageFrame(frame);
		user_page_table.entries[pte_index] = createPTEWithOptions(0, 0);
	}

	((ProcessInfo *) KERNEL_STACK_BASE)->current_brk = (void *) UP_TO_PAGE(address);
	return 0;
}




/* 
  Method: sets the lowest location not used by the program
*/

void kernelBrk(UserContext *context) {
	void *address = context->regs[0];
	
	//need some variable to keep track of lowest location not used by program
	//modify addr to be rounded up to the next multiple of PAGESIZE bytes
	//set that variable to be equal to addr
	//return ERROR if one occurs
	
	if (UP_TO_PAGE(address) >= DOWN_TO_PAGE(context->sp)) {
		TracePrintf(1, "Hey! You're trying to expand the heap into the stack!\n");
		return -1;
	}

	if(UP_TO_PAGE(address) <= ((ProcessInfo *) KERNEL_STACK_BASE)->data_start) {
		TracePrintf(1, "Hey! You're trying to expand the heap into the data area!\n");
		return -1;
	}

	//check whether we actually need to increase/decrease the current brk
	if (UP_TO_PAGE(address) == ((ProcessInfo *) KERNEL_STACK_BASE)->current_brk) return 0;

	int status = (UP_TO_PAGE(address) > ((ProcessInfo *)KERNEL_STACK_BASE)->current_brk ?
		increaseBrk(address) :
		decreaseBrk(address));

	if (status == -1) return -1;

	TracePrintf(2, "Changed the current_brk to %lX\n", 
		((ProcessInfo *)KERNEL_STACK_BASE)->current_brk);

	return 0;
}
