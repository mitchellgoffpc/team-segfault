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
#include "../process/process.h"
#include "memory.h"





/* =============================== *

  	     Implementation

 * =============================== */

/*
  If we're increasing the size of the heap, allocate some new page frames
  and set the page table entries for the heap to point to them
*/

static int increaseBrk(void *address) {
	// Figure out how many new frames we need to allocate
	long current_brk = (long)((ProcessInfo *) KERNEL_STACK_BASE)->current_brk;
	long frames_needed = (UP_TO_PAGE(address) - current_brk) >> PAGESHIFT;

	TracePrintf(2, "Increasing the user heap for process %d by %d pages\n",
		getCurrentProcess()->pid, frames_needed);

	for (int i=0; i<frames_needed; i++) {
		// Allocate a new physical page frame, then create a PTE for it
		void *frame = allocatePageFrame();
		errorIfNull(frame, "There aren't any page frames left for the heap :(\n");

		long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
		PTE entry = createPTEWithOptions(options, indexOfPage(frame));

		// Insert the new PTE into the page table
		long index = indexOfPage(current_brk - VMEM_1_BASE) + i;
		getCurrentProcess()->page_table->entries[index] = entry;
		WriteRegister(REG_TLB_FLUSH, UP_TO_PAGE(current_brk) + PAGESIZE*i);
	}

	return 0;
}




/*
  If we're shrinking the heap, free any page frames we no longer need
  and mark the corresponding page table entries as invalid
*/

static int decreaseBrk(void *address) {

	// Figure out how many frames we need to free
	long current_brk = (long)((ProcessInfo *) KERNEL_STACK_BASE)->current_brk;
	long frames_freed = (current_brk - UP_TO_PAGE(address)) >> PAGESHIFT;

	TracePrintf(2, "Decreasing the user heap for process %d by %d pages\n",
		getCurrentProcess()->pid, frames_freed);

	for (int i=0; i<frames_freed; i++) {
		// Figure out which physical frame to free
		long pte_index = indexOfPage(UP_TO_PAGE(address) - VMEM_1_BASE) + i;
		long frame_index = getCurrentProcess()->page_table->entries[pte_index].pfn;
		void *frame = pageAtIndex(frame_index);

		// Free the frame and clear the PTE
		freePageFrame(frame);
		getCurrentProcess()->page_table->entries[pte_index] = createPTEWithOptions(0, 0);
		WriteRegister(REG_TLB_FLUSH, UP_TO_PAGE(address) + PAGESIZE*i);
	}

	return 0;
}


/* 
  Method: sets the lowest location not used by the program
*/

int setProcessBrk(void *address) {
	UserContext *context = &getCurrentProcess()->user_context;
	long current_brk = (long)((ProcessInfo *) KERNEL_STACK_BASE)->current_brk;
	
	if (UP_TO_PAGE(address) >= DOWN_TO_PAGE(context->sp)) {
		TracePrintf(1, "Hey, you're trying to expand the heap into the stack!\n");
		return ERROR;
	}

	if (DOWN_TO_PAGE(address) < (long)((ProcessInfo *) KERNEL_STACK_BASE)->heap_start) {
		TracePrintf(1, "Hey, you're trying to shrink the heap into the data area!\n");
		return ERROR;
	}

	// Check whether we actually need to increase/decrease the current brk
	if (UP_TO_PAGE(address) == current_brk) return 0;

	int status = (UP_TO_PAGE(address) > current_brk ?
		increaseBrk(address) :
		decreaseBrk(address));

	checkForError(status);

	((ProcessInfo *) KERNEL_STACK_BASE)->current_brk = (void *) UP_TO_PAGE(address);
	TracePrintf(2, "Changed the current brk to %lX\n", (void *) UP_TO_PAGE(address));

	return 0;
}
