/*
  File: memory.h
  Date: 10/6/2014
  Author: Mitchell Goff
*/



#ifndef __YALNIX_MEMORY_H__
#define __YALNIX_MEMORY_H__



/* =============================== *

  	     	 Includes

 * =============================== */

#include <stdint.h>

#include "../include/hardware.h"
#include "../core/list.h"





/* =============================== *

  	      Data Structures

 * =============================== */

#define PTE_VALID 		    0x01
#define PTE_ACCESS		    0x10
#define PTE_MODIFIED        0x20
#define PTE_COPY_ON_WRITE	0x40
#define PTE_MISC_MASK       0xF0

#define PTE_PERM_READ       0x02
#define PTE_PERM_WRITE      0x04
#define PTE_PERM_EXEC       0x08
#define PTE_PERM_MASK       0x0E


struct PageTable;
struct PTE;

extern int VIRTUAL_MEMORY_ENABLED;
extern void *KERNEL_DATA;
extern void *KERNEL_BRK;
extern long PMEM_SIZE;

extern LinkedListNode frame_head;
extern struct PageTable kernel_page_table;
extern char *frc_table;

typedef struct PTE PTE;
typedef struct PageTable PageTable;


struct PTE {
    u_long valid        : 1;  /* page mapping is valid */
    u_long perm         : 3;  /* page protection bits */
    u_long misc         : 4;  /* reserved; currently unused */
    u_long pfn          : 24; /* page frame number */
};

struct PageTable {
	PTE entries[VMEM_REGION_SIZE >> PAGESHIFT];
};





/* =============================== *

  	        Macros

 * =============================== */

/*
  Some macros for testing purposes.
*/

// Transform a virtual address into a physical one.
#define v2p(addr) ({ \
  	PTE entry = kernel_page_table.entries[(long)(addr) >> PAGESHIFT]; \
  	long p_offset = ( ((long)entry.pfn) << PAGESHIFT ); \
  	(void *) (PMEM_BASE + p_offset + ((addr) & PAGEOFFSET)); })



// Transform a virtual address into a PTE index and vice versa
#define pageAtIndex(pte)  ((void *) ((long)(pte)  << PAGESHIFT))
#define indexOfPage(page) 			((long)(page) >> PAGESHIFT)


#define NUMBER_OF_FRAME_WINDOWS 3

// Get the base index of the frame window PTEs
#define frame_window_pte_base (indexOfPage(KERNEL_STACK_BASE - VMEM_BASE) - NUMBER_OF_FRAME_WINDOWS)

// Get the frame window PTE at a particular index
#define get_frame_window_pte(i) (&(kernel_page_table.entries[frame_window_pte_base + (long)(i)]))

#define frame_window_pte(i) \
    if (VIRTUAL_MEMORY_ENABLED) { WriteRegister(REG_TLB_FLUSH, (long)frame_window(i)); } \
	kernel_page_table.entries[frame_window_pte_base + (long)(i)]

// Get the address of the frame window with a particular index

// Normally, this would point to the virtual address that we're using as a window
// to a physical page frame. However, for testing purposes, this will return the
// location of the page frame within the simulated physical memory so we can write
// to it directly.
#define frame_window(i) ((void *) (KERNEL_STACK_BASE - (long)pageAtIndex(NUMBER_OF_FRAME_WINDOWS-(i))))
// #define frame_window(i) v2p(KERNEL_STACK_BASE - VMEM_BASE - ((NUMBER_OF_FRAME_WINDOWS-(i)) << PAGESHIFT))




/* =============================== *

  	         Interface

 * =============================== */

void initFrameList();
void initKernelPageTable();
void initFRCTable();


PTE createPTEWithOptions(long options, long frame_number);
void clearPageTable(PageTable *table);

void* allocatePageFrame();
void freePageFrame(void *frame);

int SetKernelBrk(void *address);



#endif
