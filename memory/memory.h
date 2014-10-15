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

// #include "hardware.h"
#include <stdint.h>




/* =============================== *

  	      Data Structures

 * =============================== */

#define PAGESIZE	0x2000
#define PAGEOFFSET	(PAGESIZE-1)
#define PAGEMASK	(~PAGEOFFSET)
#define PAGESHIFT	13

#define VMEM_REGION_SIZE    0x100000        /* 1 megabyte */

#define	VMEM_NUM_REGION		2
#define	VMEM_SIZE		(VMEM_NUM_REGION * VMEM_REGION_SIZE)

//#define	VMEM_BASE		0
#define	VMEM_0_BASE		VMEM_BASE
#define	VMEM_0_SIZE		VMEM_REGION_SIZE
// #define	VMEM_0_LIMIT	(VMEM_0_BASE + VMEM_0_SIZE)
#define	VMEM_1_BASE		VMEM_0_LIMIT
#define	VMEM_1_SIZE		VMEM_REGION_SIZE
#define	VMEM_1_LIMIT	(VMEM_1_BASE + VMEM_1_SIZE)
#define	VMEM_LIMIT		VMEM_1_LIMIT

#define KERNEL_STACK_LIMIT	VMEM_0_LIMIT
#define KERNEL_STACK_MAXSIZE	(2*PAGESIZE)
#define KERNEL_STACK_BASE \
	DOWN_TO_PAGE(KERNEL_STACK_LIMIT - KERNEL_STACK_MAXSIZE)

#define	UP_TO_PAGE(n)	(((long)(n) + PAGEOFFSET) & PAGEMASK)
#define	DOWN_TO_PAGE(n)	((long)(n) & PAGEMASK)

#define REG_TLB_FLUSH	3

#ifndef _PARAMS
#if defined(__STDC__) || defined(__cplusplus)
#define _PARAMS(ARGS) ARGS
#else
#define _PARAMS(ARGS) ()
#endif
#endif /* _PARAMS */

static inline void WriteRegister _PARAMS((int which, unsigned int value)) { }





#define PTE_VALID 		0x80000000
#define PTE_ACCESS		0x08000000
#define PTE_MODIFIED	0x04000000
#define PTE_CP_ON_WRITE	0x02000000

#define PTE_PERM_READ	0x40000000
#define PTE_PERM_WRITE	0x20000000
#define PTE_PERM_EXEC	0x10000000

#define PTE_ADDRESS		0x00FFFFFF


struct PageTable;
extern long VMEM_0_LIMIT;
extern long VMEM_BASE;
extern void *KERNEL_BRK;
extern long PMEM_BASE;
extern int VIRTUAL_MEMORY_ENABLED;
extern struct PageTable kernel_page_table;

typedef uint32_t PTE;
typedef struct PageTable PageTable;


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
  	long p_offset = (( ((long)entry) & PTE_ADDRESS) << PAGESHIFT); \
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
	WriteRegister(REG_TLB_FLUSH, (long)frame_window(i)); \
	kernel_page_table.entries[frame_window_pte_base + (long)(i)]

// Get the address of the frame window with a particular index
// #define frame_window(i) ((void *) (KERNEL_STACK_BASE - (long)pageAtIndex(NUMBER_OF_FRAME_WINDOWS-i)))

// Normally, this would point to the virtual address that we're using as a window
// to a physical page frame. However, for testing purposes, this will return the
// location of the page frame within the simulated physical memory so we can write
// to it directly.
#define frame_window(i) v2p(KERNEL_STACK_BASE - VMEM_BASE - ((NUMBER_OF_FRAME_WINDOWS-(i)) << PAGESHIFT))




/* =============================== *

  	         Interface

 * =============================== */


PTE createPTEWithOptions(long options, long frame_number);

void* allocatePageFrame();
void freePageFrame(void *frame);

void createFrameList();
void initKernelPageTable(void *text_limit);

int SetKernelBrk(void *address);


#endif
