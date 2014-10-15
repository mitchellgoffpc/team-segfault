/*
  File: memory.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  	         Includes

 * =============================== */

#include <stdlib.h>

#include "../process/process.h"
#include "../core/list.h"
#include "memory.h"




/* =============================== *

  	           Data

 * =============================== */

int VIRTUAL_MEMORY_ENABLED = 0;
void *KERNEL_DATA = 0;
void *KERNEL_BRK = 0;

LinkedListNode frame_head = linkedListNode(frame_head);
PageTable kernel_page_table;





/* =============================== *

  	       Implementation

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




/*
  Allocate a new page frame and returns its physical address.

  Note: Right now, this can return an error code, since swapping hasn't
  been implemented yet.
*/

void* allocatePageFrame() {
	if (frame_head.next == &frame_head) return 0;

	long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
	void *page_offset = (void *)frame_head.next;

	// Move the first frame window to the frame that frame_head.next points to
	frame_window_pte(0) = createPTEWithOptions(options, indexOfPage(frame_head.next));

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

	return page_offset;
}




/*
  Add a page frame to the front of the doubly linked list
*/

void freePageFrame(void *frame) {
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
}




/*
  Create a linked list of available page frames, where each node is stored at the
  bottom of a page frame in physical memory. This function can only be run before
  virtual memory is enabled.
*/

void createFrameList(long pmem_size) {
	for (long i = indexOfPage(KERNEL_BRK); i < indexOfPage(pmem_size); i++) {
		LinkedListNode *node = (LinkedListNode *) (PMEM_BASE + (long)pageAtIndex(i));

		node->prev = (i == indexOfPage(KERNEL_BRK) ? &frame_head : (LinkedListNode *) pageAtIndex(i-1));
		node->next = (i == indexOfPage(VMEM_REGION_SIZE)-1 ? &frame_head : (LinkedListNode *) pageAtIndex(i+1));
	}

	frame_head.prev = (LinkedListNode *) (pmem_size - PAGESIZE);
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

	long text_options = PTE_VALID | PTE_PERM_READ | PTE_PERM_EXEC;
	long data_options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;

	// First, map the physical pages containing the kernel text and data to the same addresses
	// in the virtual address space
	for (long i=0; i<indexOfPage(KERNEL_DATA); i++) {
		kernel_page_table.entries[i] = createPTEWithOptions(text_options, i);
	}
	for (long i=indexOfPage(KERNEL_DATA); i<indexOfPage(KERNEL_BRK); i++) {
		kernel_page_table.entries[i] = createPTEWithOptions(data_options, i);
	}

	// Leave the rest of the virtual address space unmapped, except for the stack
	for (long i=indexOfPage(KERNEL_BRK); i<indexOfPage(VMEM_REGION_SIZE); i++) {
		kernel_page_table.entries[i] = createPTEWithOptions(0, 0);
	}
	for (long i=indexOfPage(KERNEL_STACK_BASE); i<indexOfPage(KERNEL_STACK_LIMIT); i++) {
		kernel_page_table.entries[i] = createPTEWithOptions(data_options, i);
	}
}




/*
  Set the Kernel Brk
*/

int SetKernelBrk(void *address) {

	// Before virtual memory is enabled, we just keep track of the highest
 	// address the user has allocated thus far.
	if (VIRTUAL_MEMORY_ENABLED == 0) {
		KERNEL_BRK = (address > KERNEL_BRK ? address : KERNEL_BRK);
	}

	// After virtual memory is enabled, we need to ...
	else {
		
		// First, check if the user is trying to expand the heap into the stack area
		if (UP_TO_PAGE(address) >= KERNEL_STACK_BASE - VMEM_BASE) return -1;

		// Also, check whether we actually need to increase/decrease KERNEL_BRK
		if (UP_TO_PAGE(address) == (long)KERNEL_BRK) return 0;


		// If we're increasing the size of the heap, allocate some new page frames
		// and set the page table entries for the heap to point to them
		if (UP_TO_PAGE(address) > (long)KERNEL_BRK) {

			// Figure out how many new frames we need to allocate
			long frames_needed = (UP_TO_PAGE(address) - (long)KERNEL_BRK) >> PAGESHIFT;

			for (int i=0; i<frames_needed; i++) {
				// Allocate a new physical page frame, then create a PTE for it
				void *frame = allocatePageFrame();
				long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
				PTE entry = createPTEWithOptions(options, indexOfPage(frame));

				// Insert the new PTE into the page table
				long index = indexOfPage(KERNEL_BRK) + i;
				kernel_page_table.entries[index] = entry;
			}

			KERNEL_BRK = (void *) UP_TO_PAGE(address);
		}


		// If we're shrinking the heap, free any page frames we no longer need
		// and mark the corresponding page table entries as invalid
		else {

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
		}
	}

	return 0;
}