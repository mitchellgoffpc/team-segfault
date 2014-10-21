/*
  File: memory.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  	         Includes

 * =============================== */

#include <string.h>

#include "../core/list.h"
#include "memory.h"




/* =============================== *

  	           Data

 * =============================== */

void *KERNEL_BRK = 0;
long PMEM_SIZE = 0;
char *frc_table = NULL;

LinkedListNode frame_head = linkedListNode(frame_head);
PageTable kernel_page_table;





/* =============================== *

  	      Helper Functions

 * =============================== */

// Helper function to create a new Page Table Entry
PTE createPTEWithOptions(long options, long frame_number) {
	PTE entry;

	entry.valid = (options & PTE_VALID);
	entry.perm = (options & PTE_PERM_MASK) >> 1;
	entry.misc = (options & PTE_MISC_MASK) >> 4;
	entry.pfn = frame_number;

	return entry;
}



// Helper function to mark every entry in this table as invalid
void clearPageTable(PageTable *table) {
	memset(table, 0x00, indexOfPage(VMEM_REGION_SIZE) * sizeof(PTE));
}





/* =============================== *

  	    Page Frame Allocation

 * =============================== */

/*
  Allocate a new page frame and returns its physical address.

  Note: Right now, this can return an error code, since swapping hasn't
  been implemented yet.
*/

void* allocatePageFrame() {

	TracePrintf(2, "Allocating a page frame...\n");

	// Check if there are any page frames left.
	if (frame_head.next == &frame_head) {
		TracePrintf(1, "We're out of page frames!\n");
		return 0;
	}

	long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
	void *frame = (void *)frame_head.next;

	// Move the first frame window to the frame that frame_head.next points to
	frame_window_pte(0) = createPTEWithOptions(options, indexOfPage(frame_head.next));
	frc_table[ indexOfPage(frame_head.next) ] = 1;

	// If this is the last free page, set frame_head to an empty list
	if (((LinkedListNode *) frame_window(0))->next == &frame_head) {
		linkedListNodeInit(&frame_head);
	}

	// Otherwise...
	else {
		// Move the second frame window to the frame that frame_window(0)->next points to
		frame_window_pte(1) = createPTEWithOptions(options, indexOfPage(((LinkedListNode *) frame_window(0))->next));

		// Now remove the linked list node from the chain
		frame_head.next = ((LinkedListNode *) frame_window(0))->next;
		((LinkedListNode *) frame_window(1))->prev = &frame_head;
	}

	TracePrintf(2, "Allocated a page frame at %lX\n", (long)frame);

	return frame;
}




/*
  Add a page frame to the front of the doubly linked list
*/

void freePageFrame(void *frame) {

	frc_table[ indexOfPage(frame) ] -= 1;
	if (frc_table[ indexOfPage(frame) ] > 0) return;

	long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;

	// Move the first frame window to the frame that frame_head.next points to
	frame_window_pte(0) = createPTEWithOptions(options, indexOfPage(frame));

	// Now add the linked list node into the chain
	LinkedListNode *next_page = frame_head.next;
	frame_head.next = (LinkedListNode *) frame;
	((LinkedListNode *) frame_window(0))->prev = &frame_head;


	// If the list was empty before, just close it off
	if (next_page == &frame_head) {
		frame_head.prev = (LinkedListNode *) frame;
		((LinkedListNode *) frame_window(0))->next = &frame_head;
	}

	// Otherwise...
	else {
		// Move the second frame window to the frame that frame_head.next pointed to
		frame_window_pte(1) = createPTEWithOptions(options, indexOfPage(next_page));

		((LinkedListNode *) frame_window(0))->next = next_page;
		((LinkedListNode *) frame_window(1))->prev = (LinkedListNode *) frame;
	}

	TracePrintf(2, "Freed a page frame at %lX\n", (long)frame);
}





/* =============================== *

  	   Kernel Heap Allocation

 * =============================== */

/*
  If we're increasing the size of the heap, allocate some new page frames
  and set the page table entries for the heap to point to them
*/

static int increaseKernelBrk(void *address) {

	// Figure out how many new frames we need to allocate
	long frames_needed = (UP_TO_PAGE(address) - (long)KERNEL_BRK) >> PAGESHIFT;

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
		long index = indexOfPage(KERNEL_BRK) + i;
		kernel_page_table.entries[index] = entry;
	}

	KERNEL_BRK = (void *) UP_TO_PAGE(address);
	return 0;
}




/*
  If we're shrinking the heap, free any page frames we no longer need
  and mark the corresponding page table entries as invalid
*/

static int decreaseKernelBrk(void *address) {

	// Figure out how many frames we need to free
	long frames_freed = ((long)KERNEL_BRK - UP_TO_PAGE(address)) >> PAGESHIFT;

	for (int i=0; i<frames_freed; i++) {
		// Figure out which physical frame to free
		long pte_index = indexOfPage(UP_TO_PAGE(address)) + i;
		long frame_index = kernel_page_table.entries[pte_index].pfn;
		void *frame = pageAtIndex(frame_index);

		// Free the frame and clear the PTE
		freePageFrame(frame);
		kernel_page_table.entries[pte_index] = createPTEWithOptions(0, 0);
	}

	KERNEL_BRK = (void *) UP_TO_PAGE(address);
	return 0;
}




/*
  Hook for malloc() and free() to increase or decrease the size of the kernel heap
*/

int SetKernelBrk(void *address) {

	// Before virtual memory is enabled, we just keep track of the highest
 	// address the user has allocated thus far.
	if (!VIRTUAL_MEMORY_ENABLED) {
		KERNEL_BRK = (address > KERNEL_BRK ? address : KERNEL_BRK);
	}

	// After virtual memory is enabled...
	else {
		
		// First, check if the user is trying to expand the heap into the stack area
		if (UP_TO_PAGE(address) >= KERNEL_STACK_BASE - VMEM_BASE - (NUMBER_OF_FRAME_WINDOWS << PAGESHIFT)) {
			TracePrintf(1, "Hey! You're trying to expand the kernel heap into the stack!\n");
			return -1;
		}

		// Also, check whether we actually need to increase/decrease KERNEL_BRK
		if (UP_TO_PAGE(address) == (long)KERNEL_BRK) return 0;

		// If everything checks out, try to expand or shrink the heap
		int status = (UP_TO_PAGE(address) > (long)KERNEL_BRK ?
			increaseKernelBrk(address) :
			decreaseKernelBrk(address) );

		if (status == -1) return -1;
	}


	TracePrintf(2, "Changed KERNEL_BRK to %lX\n", (long)KERNEL_BRK);
	return 0;
}
