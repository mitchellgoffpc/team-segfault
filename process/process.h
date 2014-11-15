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
#include <string.h>

#include "../include/hardware.h"
#include "../memory/memory.h"
#include "../core/list.h"





/* =============================== *

          Data Structures

 * =============================== */

#define ERROR (-1)
#define KILL (-2)
#define SUCCESS 0

extern long max_pid;
extern LinkedListNode process_head;


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

  PROCESS_DEAD:    The process has been killed, and its parent has just issued
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
  The ProcessInfo struct contains some low-level information about a process and
  lets us get back to the ProcessDescriptor using just the stack pointer. Also
  includes a pointer to the bottom of the heap and data areas, as well as the
  current brk.
*/

struct ProcessInfo {
    ProcessDescriptor* descriptor;
    void *data_start;
    void *heap_start;
    void *current_brk;
};




/*
  The ProcessDescriptor struct contains all of the important information about a
  process.

  pid:          The unique ID of the process. Ranges from 0 and PID_MAX
  wake_up_time: The amount of time we need to wait for the process to wake up
  exit_status:  The state that this process exited with
  state:        The current state of the process

  info:         Some low-level information about the process. This is also a pointer
                to the Process Control Block, in case we ever need it


  parent:       A pointer to the descriptor of our parent, if it's is still around,
                or the descriptor of process 1 (init) if our parent no longer exists
  children:     The head of the list containing all of our children
  siblings:     A linked list node that can be hooked onto by our parent's child list
  
  thread_leader: A pointer to the descriptor of our thread group leader
  thread_group: The head of the list containing all of the threads in our thread_group
  thread_peers: A linked list node that can be hooked onto by our parent's thread_group list


  process_list: A linked list node that can be hooked onto by the global process list
                Useful for iterating through all the processes at once

  waitqueue:    A linked list node that can be hooked onto by a waitqueue

  page_table:   The REGION_1 page table for this process
  user_context: The UserContext for this process. We need to save this whenever we
                switch to kernel mode so we can use it later on to resume the process
  kernel_context: The KernelContext for this process. We need to save this whenever
                we switch processes so we can use it later on to resume the process
*/

struct ProcessDescriptor {
    PID pid;

    long wake_up_time;
    long exit_status;

    enum ProcessState state;
    void *pcb_frames[KERNEL_STACK_MAXSIZE >> PAGESHIFT];
    
    ProcessDescriptor* parent;
    LinkedListNode children;
    LinkedListNode siblings;

    ProcessDescriptor* thread_leader;
    LinkedListNode thread_group;
    LinkedListNode thread_peers;

    LinkedListNode process_list;
    struct WaitQueueNode *waitqueue;

    PageTable *page_table;
    UserContext user_context;
    KernelContext kernel_context;
};




/*
  The ProcessControlBlock union contains both the kernel stack for a particular
  process and a ProcessInfo struct to hold some additional info (like the PID).
*/

union ProcessControlBlock {
    ProcessInfo info;
    char stack[KERNEL_STACK_MAXSIZE]; /* 1024 for 4KB stacks */
};





/* =============================== *

              Macros

 * =============================== */

// Get the process descriptor of the currently running process
#define getCurrentProcess() \
    (((ProcessInfo *) KERNEL_STACK_BASE)->descriptor)

#define getIdleProcess() \
    elementForNode(process_head.next, ProcessDescriptor, process_list)

// Get the next available process ID
#define nextAvailablePID() \
    (max_pid < LONG_MAX ? ++max_pid : 0)




// Save the current user context into the process descriptor
#define saveUserContext() \
    memcpy(&getCurrentProcess()->user_context, context, sizeof(UserContext));

#define restoreUserContext() \
    memcpy(context, &getCurrentProcess()->user_context, sizeof(UserContext));




// Return an error code if the value is null
#define errorIfNull(value, message) \
    if (!value) { \
        TracePrintf(1, message); \
        return ERROR; \
    }

// Return a kill code if the value is null
#define killIfNull(value, message) \
    if (!value) { \
        TracePrintf(1, message); \
        return KILL; \
    }

// Halt the machine if the value is null
#define haltIfNull(value, message) \
    if (!value) { \
        TracePrintf(1, message); \
        Halt(); \
    }

// Check if a function returned an error
#define checkForError(value) \
    if (value == ERROR) { return ERROR; }






/*
  Some macros to create a new process descriptor
*/

// Dynamically initialize a new linked list node
static inline void processDescriptorInit(ProcessDescriptor *process) {
    memset(process, 0x00, sizeof(ProcessDescriptor));
    
    process->pid = nextAvailablePID();
    process->state = PROCESS_RUNNING;

    linkedListNodeInit(&process->children);
    linkedListNodeInit(&process->siblings);
    linkedListNodeInit(&process->thread_group);
    linkedListNodeInit(&process->thread_peers);

    linkedListNodeInit(&process->process_list);
}





/* =============================== *

            Interface

 * =============================== */

KernelContext* cloneKernelContext(KernelContext *context, void *a, void *b);
KernelContext* switchKernelContext(KernelContext *context, void *a, void *b);
KernelContext* killKernelContext(KernelContext *context, void *a, void *b);
void schedule();


int forkProcess();
int loadProgram(char *name, char *args[]);


ProcessDescriptor* createProcessDescriptor();
void setCopyOnWrite(PageTable *table, int is_child);
void freeAddressSpace(ProcessDescriptor *process);
int delayProcess(int ticks);
int waitForPID(unsigned long pid, int *status);


int createThread();
int joinThread(unsigned long thread_id);


void killCurrentProcess(int status);
void killProcess(ProcessDescriptor *process, int status);
void releaseProcess(ProcessDescriptor *process);



#endif
