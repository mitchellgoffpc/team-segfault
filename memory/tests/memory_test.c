#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../memory.h"


void testFrameList() {
	void *virtual_memory = malloc(VMEM_REGION_SIZE + PAGESIZE);
	void *physical_memory = malloc(VMEM_REGION_SIZE + PAGESIZE);
	if (!virtual_memory || !physical_memory) return;

	// Create an artificial memory space to do testing in
	VMEM_BASE = UP_TO_PAGE(virtual_memory);
	VMEM_0_LIMIT = VMEM_BASE + VMEM_REGION_SIZE;
	PMEM_BASE = UP_TO_PAGE(physical_memory);
	KERNEL_BRK = 0;

	SetKernelBrk((void *)0x4000);

	printf("Virtual Memory Start: %lX\n", VMEM_BASE);
	printf("Virtual Memory End: %lX\n", VMEM_LIMIT);
	printf("\n");

	printf("Physical Memory Start: %lX\n", PMEM_BASE);
	printf("Physical Memory End: %lX\n", PMEM_BASE + VMEM_REGION_SIZE);
	printf("\n");


	// Initialize the memory
	createFrameList();
	initKernelPageTable((void *)0);

	printf("PTE 0: %lX\n", (long)kernel_page_table.entries[0]);
	printf("PTE 1: %lX\n", (long)kernel_page_table.entries[1]);
	printf("\n");


	// Allocate some page tables
	void *frame1 = allocatePageFrame();
	printf("Allocated page frame %lX\n", (long)frame1);

	void *frame2 = allocatePageFrame();
	printf("Allocated page frame %lX\n", (long)frame2);
	printf("\n");
	

	// Free the page tables
	freePageFrame(frame1);
	printf("Freed page frame %lX\n", (long)frame1);
	
	freePageFrame(frame2);
	printf("Freed page frame %lX\n", (long)frame2);
	printf("\n");


	// Increase the kernel brk
	VIRTUAL_MEMORY_ENABLED = 1;
	SetKernelBrk((void *)0x8000);

	printf("PTE 2: %lX\n", (long)kernel_page_table.entries[2]);
	printf("PTE 3: %lX\n", (long)kernel_page_table.entries[3]);
	printf("\n");


	// Decrease the kernel brk
	SetKernelBrk((void *)0x5000);

	printf("PTE 2: %lX\n", (long)kernel_page_table.entries[2]);
	printf("PTE 3: %lX\n", (long)kernel_page_table.entries[3]);
	printf("\n");


	free(virtual_memory);
	free(physical_memory);
}


int main() {
	testFrameList();

	printf("All tests passed!\n");
	return 0;
}