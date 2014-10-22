/*
  File: fork.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/


/* =============================== *

             Includes

 * =============================== */

#include <stdlib.h>
#include <string.h>

#include "../include/hardware.h"
#include "../include/load_info.h"
#include "../memory/memory.h"
#include "process.h"





/* =============================== *

           Implementation

 * =============================== */

/*
  Fork the existing address space
*/

int forkProcess() {
    TracePrintf(1, "Getting read to fork...\n");
    ProcessDescriptor *parent = getCurrentProcess();

    // Try to allocate space for the new process descriptor
    ProcessDescriptor *process = createProcessDescriptor();
    errorIfNull(process, "There's not enough space for a new process descriptor!\n");

    // Try to allocate space for the new page table
    PageTable *table = (PageTable *) malloc(sizeof(PageTable));
    errorIfNull(table, "There's not enough space for a new page table!\n");
    memcpy(table, parent->page_table, sizeof(PageTable));
    process->page_table = table;


    // Make a copy of the parent's stack, since we know we'll need that right away
    int stack_base = ((long)getCurrentProcess()->user_context.sp - VMEM_1_BASE) >> PAGESHIFT;
    for (int i=stack_base; i<indexOfPage(VMEM_REGION_SIZE); i++) {
        PTE old_entry = parent->page_table->entries[i];
        if (!old_entry.valid) continue;

        long options = PTE_VALID | (old_entry.perm << 1) | (old_entry.misc << 4);
        void *frame = allocatePageFrame();
        process->page_table->entries[i] = createPTEWithOptions(options, indexOfPage(frame));

        frame_window_pte(0) = createPTEWithOptions(PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE, indexOfPage(frame));
        memcpy(frame_window(0), (void *)(VMEM_1_BASE + pageAtIndex(i)), PAGESIZE);
    }


    // Copy the parent's user context
    memcpy(&process->user_context, &parent->user_context, sizeof(UserContext));

    // Update the process descriptor and tell the machine where the REGION_1
    // page table is located
    long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
    frame_window_pte(0) = createPTEWithOptions(options, indexOfPage(process->pcb_frames[0]));
    ((ProcessInfo *) frame_window(0))->descriptor = process;

    // Set up the linked lists connecting the parent to the child
    process->parent = parent;
    addLastNode(&process->siblings, &parent->children);
    addLastNode(&process->process_list, &process_head);

    // Finally, create the new process by cloning the current kernel context and stack
    KernelContextSwitch(cloneKernelContext, (void *) parent, (void *) process);
    setCopyOnWrite(getCurrentProcess()->page_table, getCurrentProcess() == process);

    TracePrintf(1, "Finished forking!\n");

    return (getCurrentProcess() == parent ? process->pid : 0);
}
