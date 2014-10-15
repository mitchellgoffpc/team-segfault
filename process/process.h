/*
  File: process.h
  Date: 10/6/2014
  Author: Mitchell Goff
*/

#ifndef __YALNIX_PROCESS_H__
#define __YALNIX_PROCESS_H__



/* =============================== *

  			 Includes

 * =============================== */

#include <limits.h>
#include "../memory/memory.h"
#include "list.h"




/* =============================== *

  			 Constants

 * =============================== */

// int MAX_PID = INT_MAX;




/* =============================== *

  		  Data Structures

 * =============================== */

struct ProcessInfo;
struct ProcessDescriptor;
struct WaitQueueNode;

typedef struct ProcessInfo ProcessInfo;
typedef struct ProcessDescriptor ProcessDescriptor;

typedef unsigned int PID;




/*
  The ProcessState enum describes all the possible states that a process
  can be in.

  PROCESS_RUNNING: The process is currently running or waiting to be run.
  PROCESS_WAITING: The process is sleeping until some condition becomes
  				   true. This is useful when waiting for a hardware interrupt
                   or when putting a process on a waitqueue

  PROCESS_STOPPED: The process has been stopped. This occurs when the process
  				   is sent a SIG_STOP or SIG_KSTOP signal, and is useful for
  				   job control in the shell, amongst other things

  PROCESS_ZOMBIE:  The process has been killed, but it's parent hasn't yet
  				   issued a waitpid()-like syscall to get information from
  				   the child's ProcessDescriptor, so we can't get rid of the
  				   PCB yet.

  PROCESS_DEAD:	   The process has been killed, and its parent has just issued
  				   a waitpid()-like syscall to get information from the child's
  				   ProcessDescriptor. This state is used to avoid race
  				   conditions caused by multiple threads issuing waitpid()
  				   -like syscalls on the same process.
*/

enum ProcessState {
    PROCESS_RUNNING=0,
	PROCESS_WAITING=1,
	PROCESS_STOPPED=2,
	PROCESS_ZOMBIE=4,
	PROCESS_DEAD=8
};




/*
  The ProcessInfo struct contains some low-level information about a
  process. Currently just a placeholder that lets us get back to the
  ProcessDescriptor from the stack pointer.
*/

struct ProcessInfo {
    ProcessDescriptor* descriptor;
};




/*
  The ProcessDescriptor struct contains all of the important information about a
  process.

  pid: 		The unique ID of the process. Ranges from 0 and PID_MAX.
  state:	The current state of the process
  info:		Some low-level information about the process. This is also a pointer
			to the Process Control Block, in case we ever need it.
  

  parent:	A pointer to the descriptor of our parent, if it's is still around,
  			or the descriptor of process 1 (init) if our parent no longer exists.
  children:	The head of the list containing all of our children
  
  siblings:	A linked list node that can be hooked onto by our parent's child list.
  group_leader_pid:	 The PID of our process group's leader
  thread_leader_pid: The PID of our thread group's leader


  process_list: A linked list node that can be hooked onto by the global process list.
  				Useful for iterating through all the processes at once.

  waitqueue: 	A linked list node that can be hooked onto by a waitqueue. Useful


*/

struct ProcessDescriptor {
	PID pid;

	enum ProcessState state;
	ProcessInfo* info;
	
	ProcessDescriptor* parent;
	LinkedListNode children;
	LinkedListNode siblings;

	PID group_leader_pid;
	PID thread_leader_pid;

	LinkedListNode process_list;
	struct WaitQueueNode *waitqueue;

  PageTable *page_table;
};




/*
  The ProcessControlBlock union contains both the kernel stack for a particular
  process and a ProcessInfo struct to hold some additional info (like the PID).
*/

union ProcessControlBlock {
	ProcessInfo info;
	unsigned long stack[2048]; /* 1024 for 4KB stacks */
};



#endif
