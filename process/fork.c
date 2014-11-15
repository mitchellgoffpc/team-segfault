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


// Try to allocate space for the new page table
int createPageTable(ProcessDescriptor *child, ProcessDescriptor *parent) {
    PageTable *table = (PageTable *) malloc(sizeof(PageTable));
    errorIfNull(table, "There's not enough space for a new page table!\n");
    memcpy(table, parent->page_table, sizeof(PageTable));
    child->page_table = table;
}


// Make a copy of the parent's stack for the child, since we know we'll need that right away
int copyParentStack(ProcessDescriptor *child, ProcessDescriptor *parent) {
    int stack_base = ((long)getCurrentProcess()->user_context.sp - VMEM_1_BASE) >> PAGESHIFT;
    for (int i=stack_base; i<indexOfPage(VMEM_REGION_SIZE); i++) {
        PTE old_entry = parent->page_table->entries[i];
        if (!old_entry.valid) continue;

        long options = PTE_VALID | (old_entry.perm << 1) | (old_entry.misc << 4);
        void *frame = allocatePageFrame();
        child->page_table->entries[i] = createPTEWithOptions(options, indexOfPage(frame));

        frame_window_pte(0) = createPTEWithOptions(PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE, indexOfPage(frame));
        memcpy(frame_window(0), (void *)(VMEM_1_BASE + (long) pageAtIndex(i)), PAGESIZE);
    }
}


// Make a copy of the rest of the parent's writeable address space (useful if the parent is a thread)
int copyParentData(ProcessDescriptor *child, ProcessDescriptor *parent) {
    int stack_base = ((long)getCurrentProcess()->user_context.sp - VMEM_1_BASE) >> PAGESHIFT;
    for (int i=0; i<stack_base; i++) {
        PTE old_entry = parent->page_table->entries[i];
        long options = PTE_VALID | (old_entry.perm << 1) | (old_entry.misc << 4);

        if (!old_entry.valid || !(options & PTE_PERM_WRITE)) continue;
        
        void *frame = allocatePageFrame();
        child->page_table->entries[i] = createPTEWithOptions(options, indexOfPage(frame));

        frame_window_pte(0) = createPTEWithOptions(PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE, indexOfPage(frame));
        memcpy(frame_window(0), (void *)(VMEM_1_BASE + (long) pageAtIndex(i)), PAGESIZE);
    }
}


// Increase the FRC entries of any shared pages
int increaseFRCEntries(ProcessDescriptor *child, ProcessDescriptor *parent) {
    int stack_base = ((long)getCurrentProcess()->user_context.sp - VMEM_1_BASE) >> PAGESHIFT;
    for (int i=0; i<stack_base; i++) {
        PTE old_entry = child->page_table->entries[i];
        if (!old_entry.valid) continue;
        frc_table[old_entry.pfn]++;
    }
}


// Create a new user context for the child process
int createUserContext(ProcessDescriptor *child, ProcessDescriptor *parent) {
    memcpy(&child->user_context, &parent->user_context, sizeof(UserContext));

    // Update the process info struct so it points to the process descriptor
    long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
    frame_window_pte(0) = createPTEWithOptions(options, indexOfPage(child->pcb_frames[0]));
    ProcessInfo *child_info =  (ProcessInfo *) frame_window(0);
    ProcessInfo *parent_info = (ProcessInfo *) KERNEL_STACK_BASE;

    child_info->descriptor = child;
    child_info->data_start = parent_info->data_start;
    child_info->heap_start = parent_info->heap_start;
    child_info->current_brk = parent_info->current_brk;
}




/*
  Fork the current process
*/

int forkProcess() {
    TracePrintf(1, "Getting read to fork...\n");
    ProcessDescriptor *parent = getCurrentProcess();

    // Make sure the parent isn't a thread (for simplicity)
    if (parent->thread_leader > 0) {
        TracePrintf(1, "Sorry, threads can't fork their own processes!\n");
        return ERROR;
    }

    // Try to allocate space for the new process descriptor
    ProcessDescriptor *child = createProcessDescriptor();
    errorIfNull(child, "There's not enough space for a new process descriptor!\n");


    // Call the helper functions
    checkForError(createPageTable(child, parent));
    checkForError(copyParentStack(child, parent));
    checkForError(createUserContext(child, parent));

    // Set the copy-on-write bits
    setCopyOnWrite(parent->page_table, 0);
    setCopyOnWrite(child->page_table, 1);


    // Set up the linked lists connecting the parent to the child
    child->parent = parent;
    addLastNode(&child->siblings, &parent->children);
    addLastNode(&child->process_list, &process_head);

    // Finally, create the new process by cloning the current kernel context and stack
    KernelContextSwitch(cloneKernelContext, (void *) parent, (void *) child);
    TracePrintf(1, "Finished forking!\n");
    TracePrintf(1, "Current BRK: %lX\n", (long)((ProcessInfo *) KERNEL_STACK_BASE)->current_brk);
    
    return (getCurrentProcess() == parent ? child->pid : 0);
}




/*
  Spawn a new thread. This is similar to fork, except we set up the process descriptor
  differently and don't mark the shared PTEs as copy-on-write.
*/

int createThread() {
    TracePrintf(1, "Getting read to create thread...\n");
    ProcessDescriptor *parent = getCurrentProcess();

    // Make sure the parent isn't a thread (for simplicity)
    if (parent->thread_leader > 0) {
        TracePrintf(1, "Sorry, threads can't create their own sub-threads!\n");
        return ERROR;
    }

    // Try to allocate space for the new process descriptor
    ProcessDescriptor *child = createProcessDescriptor();
    errorIfNull(child, "There's not enough space for a new process descriptor!\n");


    // Call the helper functions
    checkForError(createPageTable(child, parent));
    checkForError(copyParentStack(child, parent));
    checkForError(createUserContext(child, parent));
    increaseFRCEntries(child, parent);


    // Set up the linked lists connecting the parent to the child
    child->thread_leader = parent;
    addLastNode(&child->thread_peers, &parent->thread_group);
    insertNode(&child->process_list, &parent->process_list);

    // Finally, create the new process by cloning the current kernel context and stack
    KernelContextSwitch(cloneKernelContext, (void *) parent, (void *) child);
    TracePrintf(1, "Finished creating thread!\n");
    return (getCurrentProcess() == parent ? child->pid : 0);
}
