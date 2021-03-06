/*
  File: process.c
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
#include "../core/list.h"
#include "process.h"





/* =============================== *

  		  Data Structures

 * =============================== */

LinkedListNode process_head = linkedListNode(process_head);
long max_pid = 0;




/* =============================== *

  		  Implementation

 * =============================== */

/*
  Deallocate all of the page frames that this process is currently using.
*/

void freeAddressSpace(ProcessDescriptor *process) {
    PageTable *page_table = process->page_table;

    // Go through the page table and free any valid page frames
    for (int i=0; i<indexOfPage(VMEM_REGION_SIZE); i++) {
        PTE entry = page_table->entries[i];
        if (!entry.valid) continue;
        freePageFrame((void *) pageAtIndex(entry.pfn));
    }

    // Clear all entries in the page table
    clearPageTable(page_table);
}




/*
  Mark all writeable entries in a page table as copy-on-write
*/

void setCopyOnWrite(PageTable *table, int is_child) {

    TracePrintf(3, "Setting copy-on-write bit\n");

    int stack_base = ((long)getCurrentProcess()->user_context.sp - VMEM_1_BASE) >> PAGESHIFT;
    for (int i=0; i<stack_base; i++) {
        PTE old_entry = table->entries[i];
        if (!old_entry.valid) continue;

        // If the write bit is set, clear it and set the copy on write bit
        long options = PTE_VALID | (old_entry.perm << 1) | (old_entry.misc << 4);
        if (options & PTE_PERM_WRITE) {
            options = (options & ~PTE_PERM_WRITE) | PTE_COPY_ON_WRITE;
        }

        // If I'm the child, then I need to increment the frc for any writeable pages
        if (is_child) {
            frc_table[old_entry.pfn]++;
        }

        table->entries[i] = createPTEWithOptions(options, old_entry.pfn);
    }

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
}




/*
  Create a new process descriptor and PCB
*/

ProcessDescriptor* createProcessDescriptor() {
 	TracePrintf(2, "Creating a new process descriptor...\n");

	// Try to allocate space for a new process descriptor
    ProcessDescriptor *process = (ProcessDescriptor *) malloc(sizeof(ProcessDescriptor));
    if (!process) return 0;

    // Initialize it and check if there are any available PIDs left
    processDescriptorInit(process);
    if (process->pid == 0) { free(process); return 0; }

    // Try to allocate space for a new process control block
    for (int i=0; i<indexOfPage(KERNEL_STACK_MAXSIZE); i++) {
        process->pcb_frames[i] = allocatePageFrame();
        
        // If there wasn't enough room, free any page frames we've already allocated
        if (!process->pcb_frames[i]) {
        	for (int j=i; j>=0; j--) { freePageFrame(process->pcb_frames[j]); }
        	free(process);
        	return 0;
        }
    }

    // Create the ProcessInfo struct at the bottom of the stack
    long options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
    frame_window_pte(0) = createPTEWithOptions(options, indexOfPage(process->pcb_frames[0]));
    ((ProcessInfo *) frame_window(0))->descriptor = process;

    TracePrintf(2, "Successfully created process descriptor!\n");
    return process;
}




/*  
  Delay the calling process for the given number of clock ticks
*/

int delayProcess(int ticks) {
    if (ticks == 0) return SUCCESS;

    if (ticks < 0) {
        TracePrintf(1, "Hey, we can't go back in time!\n");
        return ERROR;
    }

    // Block the calling process until so many clock interrupts have occurred
    getCurrentProcess()->wake_up_time = elapsed_clock_ticks + ticks;
    schedule();

    return SUCCESS;
}




/*
  Wait for a child to exit before returning
*/

int waitForPID(unsigned long pid, int *status) {
    ProcessDescriptor *current;

    // Make sure we actually have some children
    if (listIsEmpty(&getCurrentProcess()->children)) return -1;

    // Loop through our children until we find a zombie
    while (1) {
        forEachElement(current, &getCurrentProcess()->children, siblings) {
            if (current->state == PROCESS_ZOMBIE && (current->pid == pid || pid == 1)) goto done;
        }
        schedule();
    }

    // Then return its status
    done:
    *status = current->exit_status;
    releaseProcess(current);
    return 0;
}




/*
  Wait for a particular child thread to exit before returning
*/

int joinThread(unsigned long thread_id) {
    ProcessDescriptor *current;

    // Make sure we actually have some child threads
    if (listIsEmpty(&getCurrentProcess()->thread_group)) return -1;

    // Loop through our child threads until we find a zombie
    while (1) {
        forEachElement(current, &getCurrentProcess()->thread_group, thread_peers) {
            if (current->state == PROCESS_ZOMBIE && current->pid == thread_id) goto done;
        }
        schedule();
    }

    // Then return its status
    done:
    releaseProcess(current);
    return 0;
}
