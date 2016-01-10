team-segfault
=============

/* =============================== *

          Project Notes

 * =============================== */
 
This is a project I worked on in my OS class in winter of 2015. The goal of the project was to create a miniature but functional operating system that would run on the "yalnix" emulator, which provides some utilities to help with bootstrapping, context switches and loading programs. Besides the basic requirements, we built in some additional features such as copy-on-write and lightweight processes for threading.





/* =============================== *

            Directories

 * =============================== */

Apps:

- idle.c: This is the userland program that is loaded by KernelStart. At the moment it just fork/execs to start running "test.c".

- test.c: This userland program is just to test the exec function, and to provide a visual representation of the scheduler in action.



Core:

- list.h: This header file includes the macros, functions and data types needed to implement doubly linked lists, which are used in many different capacities throughout the kernel. Most of this code was written specifically for this project, with the exception of the "containerOf" macro at the beginning of the file (which was borrowed from the linux source).



Include:
	
- yalnix.h, hardware.h, etc: The header files needed by the kernel to interact with the hardware



Init:

- init.c: Implements KernelStart and KernelSetData, as well as some other functions which set up the interrupt vector and load the "idle" program into memory.

- init_memory.c: Some helper functions called by KernelStart to set up the page table for REGION 0, the frame reference count table (for the copy-on-write implementation), and the frame list (a linked list of available page frames, so we can allocate a new frame in constant time).



Memory:

- memory.c: Defines several functions to help with memory management, including SetKernelBrk, allocatePageFrame/freePageFrame, and handleMemoryTrap (handles copy-on-write and stack allocation)

- brk.c: Defines some functions that are used by the Brk syscall to increase/decrease the size of the user's heap



Process:

- fork.c: Implements forkProcess, which is called by the Fork syscall to create a duplicate of the current process

- kill.c: Implements killProcess, which is called by the Exit syscall to free all data structures in use by a process

- load.c: Implements loadProgram (based on template.c), which is called by the Exec syscall to overwrite the current process's address space with a new program.

- process.c: A bunch of miscellaneous functions to help with managing processes.

- switch.c: Implements schedule() to switch contexts every time the kernel recieves a TRAP_CLOCK, and defines some functions to help switch contexts or clone the current one (useful for fork)



Traps:

- traps.c: Implements the handlers for each of the traps in the interrupt vector. Most of these are simply wrappers to other functions.




/* =============================== *

           Data Structures

 * =============================== */

Just a few notes about some of the data structures being used by the kernel


ProcessDescriptor: These structs, accessible via a global linked list, contain the most important information about each process. Every process and thread has their own ProcessDescriptor

ProcessInfo: This is a small struct stored at the bottom of the Process Control Block; in fact, the PCB is defined as a union between this struct and an 8k stack. This struct has two important functions: Firstly, it contains a pointer to the associated ProcessDescriptor struct, which makes accessing the ProcessDescriptor very simple (see the getCurrentProcess() macro). Secondly, it contains some important information about the location of the userland text, data and heap segments.

LinkedListNode: See above




/* =============================== *

         Checkpoint Notes

 * =============================== */

 As far as I can tell, the requirements for this week's checkpoint (and most of next week's) have been implemented. At the moment, neither init.c and test.c is using malloc() or Delay(), but both functions have been tested and seem to be working properly. There is still a bit of error checking left to implement for the Brk() syscall, but I'm putting that off until the Kill() function has been implemented.
