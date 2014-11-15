/*
  File: kill.c
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
  Kill the current process and switch to a new one
*/

void killProcess(ProcessDescriptor *process, int status) {
	if (process->state == PROCESS_ZOMBIE || process->state == PROCESS_DEAD) return;
	TracePrintf(1, "Exiting Process %d\n", process->pid);
	

	// Figure out which process to run next
	int should_switch_processes = (process == getCurrentProcess());
	LinkedListNode *node = &process->process_list;
	ProcessDescriptor *new_process;
	while (should_switch_processes) {
		node = node->next;
		if (node == &process_head) continue;

		new_process = elementForNode(node, ProcessDescriptor, process_list);
		if (new_process->state != PROCESS_RUNNING) continue;
		if (new_process->wake_up_time > elapsed_clock_ticks) continue;
		
		break;
	}


	// Free any data structures we've allocated for this process
	freeAddressSpace(process);
	free(process->page_table);

	// Free any children who have exited, and give the rest to our parent
	ProcessDescriptor *current;
	forEachElement(current, &process->children, siblings) {
        if (current->state == PROCESS_ZOMBIE)
            releaseProcess(current);
    	else current->parent = process->parent;
    }
    spliceLinkedLists(&process->children, &process->parent->children);

    // Kill any threads we might have spawned
	forEachElement(current, &process->thread_group, thread_peers) {
        killProcess(current, -1);
        releaseProcess(current);
    }


	// Modify the process descriptor
	getCurrentProcess()->state = PROCESS_ZOMBIE;
	getCurrentProcess()->exit_status = status;

	if (should_switch_processes) {
		KernelContextSwitch(killKernelContext, process, new_process);
	}
}




/*
  Wrapper for killProcess to default to the currently running process
*/

void killCurrentProcess(int status) {
	killProcess(getCurrentProcess(), status);
}




/*
  Release the process descriptor of a zombie process
*/

void releaseProcess(ProcessDescriptor *process) {
	ProcessDescriptor *current;

	// Go through our children and release any zombies
	forEachElement(current, &process->children, siblings) {
		if (current->state == PROCESS_ZOMBIE) {
			releaseProcess(current);
		}
		current->parent = process->parent;
	}

	// Remove the process from any lists
	spliceLinkedLists(&process->children, &getIdleProcess()->children);
	removeNode(&process->children);
	removeNode(&process->siblings);
	
	removeNode(&process->process_list);

	// Free the process descriptor
	free(process);
}
