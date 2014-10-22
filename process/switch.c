/*
  File: switch.c
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
#include "../traps/traps.h"
#include "process.h"





/* =============================== *

           Implementation

 * =============================== */

/*
  Clone the kernel context into another process descriptor
*/

KernelContext* cloneKernelContext(KernelContext *context, void *a, void *b) {
	ProcessDescriptor *pa = (ProcessDescriptor *) a, *pb = (ProcessDescriptor *) b;

	memcpy(&pa->kernel_context, context, sizeof(KernelContext));
	memcpy(&pb->kernel_context, context, sizeof(KernelContext));

	// We're about to copy the parent's stack onto the child's, so we need to make
	// sure we keep a copy of the child's process info
	long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
	frame_window_pte(0) = createPTEWithOptions(options, indexOfPage(pb->pcb_frames[0]));
	ProcessInfo info = *((ProcessInfo *) frame_window(0));

	// Clone the current kernel stack
	for (int i=0; i<indexOfPage(KERNEL_STACK_MAXSIZE); i++) {
	    frame_window_pte(i) = createPTEWithOptions(options, indexOfPage(pb->pcb_frames[i]));
	    memcpy(frame_window(i), (void *)(KERNEL_STACK_BASE + PAGESIZE*i), PAGESIZE);
	}

	// Now we just have to restore the child's process info
	frame_window_pte(0) = createPTEWithOptions(options, indexOfPage(pb->pcb_frames[0]));
	*((ProcessInfo *) frame_window(0)) = info;

	return context;
}




/*
  Switch to a new kernel context
*/

KernelContext* switchKernelContext(KernelContext *context, void *a, void *b) {
	ProcessDescriptor *pa = (ProcessDescriptor *) a, *pb = (ProcessDescriptor *) b;
	if (pa != getCurrentProcess()) return context;

	// Swap the current kernel context
	memcpy(&pa->kernel_context, context, sizeof(KernelContext));

	// Remap the kernel stack
	for (int i=0; i<indexOfPage(KERNEL_STACK_MAXSIZE); i++) {
		long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
		int index = indexOfPage(KERNEL_STACK_BASE) + i;
		kernel_page_table.entries[index] = createPTEWithOptions(options, indexOfPage(pb->pcb_frames[i]));
	}

	WriteRegister(REG_PTBR1, (long)pb->page_table);
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

	return &pb->kernel_context;
}




/*
  Schedule a new process to run
*/

void schedule() {

	// Figure out which process to run next
	LinkedListNode *node = &getCurrentProcess()->process_list;
	ProcessDescriptor *new_process;
	while (1) {
		node = node->next;
		if (node == &process_head) continue;

		new_process = elementForNode(node, ProcessDescriptor, process_list);
		if (new_process->state != PROCESS_RUNNING) continue;
		if (new_process->wake_up_time > elapsed_clock_ticks) continue;
		
		break;
	}

	KernelContextSwitch(switchKernelContext, getCurrentProcess(), new_process);
}
