/*
  File: load.c
  Date: 10/6/2014
  Author: Emily Holt & Mitchell Goff
*/


/* =============================== *

            Includes

 * =============================== */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/hardware.h"
#include "../include/load_info.h"
#include "../memory/memory.h"
#include "process.h"





/* =============================== *

              Macros

 * =============================== */

#define errorIfNull(var) \
    if ((var) == NULL) { \
        close(fd); \
        return ERROR; \
    }

#define killIfNull(var) \
    if ((var) == NULL) { \
        close(fd); \
        return KILL; \
    }





/* =============================== *

           Implementation

 * =============================== */

/*
  Deallocate all of the page frames that this process is currently using.
*/

void freeAddressSpace() {
    ProcessDescriptor *process = getCurrentProcess();
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
 *    Load a program into an existing address space. The program comes from
 *    the Linux file named "name", and its arguments come from the array at
 *    "args", which is in standard argv format. The argument "proc" points
 *    to the process or PCB structure for the process into which the program
 *    is to be loaded.
 */

int loadProgram(char *name, char *args[]) {
    
    int fd;
    int (*entry)();
    struct load_info li;
    long segment_size;
    char *argbuf;
    ProcessDescriptor *process = getCurrentProcess();


    // Open the executable file and do some error checking
    if ((fd = open(name, O_RDONLY)) < 0) {
        TracePrintf(0, "LoadProgram: can't open file '%s'\n", name);
        return ERROR;
    }

    if (LoadInfo(fd, &li) != LI_NO_ERROR) {
        TracePrintf(0, "LoadProgram: '%s' not in Yalnix format\n", name);
        close(fd);
        return (-1);
    }

    if (li.entry < VMEM_1_BASE) {
        TracePrintf(0, "LoadProgram: '%s' not linked for Yalnix\n", name);
        close(fd);
        return ERROR;
    }



    // Figure out in what REGION_1 pages the different program sections
    // start and end

    long text_pg1 = (li.t_vaddr - VMEM_1_BASE) >> PAGESHIFT;
    long data_pg1 = (li.id_vaddr - VMEM_1_BASE) >> PAGESHIFT;
    long data_npg = li.id_npg + li.ud_npg;

    

    // Figure out how many bytes are needed to hold the arguments on
    // the new stack that we are building. Also count the number of
    // arguments, to become the argc that the new "main" gets called with.
    
    long i, size = 0;
    for (i=0; args[i] != NULL; i++) {
        TracePrintf(3, "counting arg %d = '%s'\n", i, args[i]);
        size += strlen(args[i]) + 1;
    }
    long argcount = i;

    TracePrintf(2, "LoadProgram: argsize %d, argcount %d\n", size, argcount);
    

    
    // The arguments will get copied starting at "cp", and the argv
    // pointers to the arguments (and the argc value) will get built
    // starting at "cpp". The value for "cpp" is computed by subtracting
    // off space for the number of arguments (plus 3, for the argc value,
    // a NULL pointer terminating the argv pointers, and a NULL pointer
    // terminating the envp pointers) times the size of each,
    // and then rounding the value *down* to a double-word boundary.

    char *cp = ((char *)VMEM_1_LIMIT) - size;

    char **cpp = (char **)
        (((long)cp - 
            ((argcount + 3 + POST_ARGV_NULL_SPACE) *sizeof (void *))) 
         & ~7);

    
    // Compute the new stack pointer, leaving INITIAL_STACK_FRAME_SIZE bytes
    // reserved above the stack pointer, before the arguments.
    char *cp2 = (caddr_t)cpp - INITIAL_STACK_FRAME_SIZE;
    
    TracePrintf(1, "prog_size %d, text %d data %d bss %d pages\n",
	            li.t_npg + data_npg, li.t_npg, li.id_npg, li.ud_npg);


    // Compute how many pages we need for the stack
    long stack_npg = (VMEM_1_LIMIT - DOWN_TO_PAGE(cp2)) >> PAGESHIFT;
    long stack_pg1 = indexOfPage(VMEM_1_LIMIT) - stack_npg;
    
    TracePrintf(1, "LoadProgram: heap_size %d, stack_size %d\n",
	            li.t_npg + data_npg, stack_npg);


    // Leave at least one page between heap and stack
    if (stack_npg + data_pg1 + data_npg >= MAX_PT_LEN) {
        close(fd);
        return ERROR;
    }



    


    // This completes all the checks before we proceed to actually load
    // the new program. From this point on, we are committed to either
    // loading succesfully or killing the process. 

    // Set the new stack pointer value in the process's exception frame.
    process->context->sp = cp2;

    
    // Now save the arguments in a separate buffer in region 0, since
    // region 1 doesn't exist yet for this process
    cp2 = argbuf = (char *) malloc(size);
    errorIfNull(argbuf);

    for (i=0; args[i] != NULL; i++) {
        TracePrintf(3, "saving arg %d = '%s'\n", i, args[i]);
        strcpy(cp2, args[i]);
        cp2 += strlen(cp2) + 1;
    }


    
    // Now set up the page table for the process so that we can read the
    // program into memory. Start by freeing any page frames we're currently
    // using and clearing the page table.
    int text_options = PTE_VALID | PTE_PERM_READ | PTE_PERM_EXEC;
    int data_options = PTE_VALID | PTE_PERM_READ | PTE_PERM_WRITE;
    freeAddressSpace();

    ((ProcessInfo *) KERNEL_STACK_BASE)->data_start = pageAtIndex(data_pg1);
    ((ProcessInfo *) KERNEL_STACK_BASE)->heap_start = pageAtIndex(data_pg1 + data_npg);
    ((ProcessInfo *) KERNEL_STACK_BASE)->current_brk = pageAtIndex(data_pg1 + data_npg);



    // Now allocate some physical pages and map them to the right places
    // in text, data and stack segments, marking everything as writable
    for (i=0; i<li.t_npg; i++) {
        void *frame = allocatePageFrame(); killIfNull(frame);
        PTE entry = createPTEWithOptions(data_options, indexOfPage(frame));
        process->page_table->entries[text_pg1 + i] = entry;
    }
  
    for (i=0; i<data_npg; i++) {
        void *frame = allocatePageFrame(); killIfNull(frame);
        PTE entry = createPTEWithOptions(data_options, indexOfPage(frame));
        process->page_table->entries[data_pg1 + i] = entry;
    }

    for (i=0; i<stack_npg; i++) {
        void *frame = allocatePageFrame(); killIfNull(frame);
        PTE entry = createPTEWithOptions(data_options, indexOfPage(frame));
        process->page_table->entries[stack_pg1 + i] = entry;
    }

    // Flush the TLB for region 1 so we can write to these pages
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);




    // Read the text from the file into memory.
    lseek(fd, li.t_faddr, SEEK_SET);
    segment_size = li.t_npg << PAGESHIFT;

    if (read(fd, (void *) li.t_vaddr, segment_size) != segment_size) {
        close(fd);
        return KILL;
    }


    // Read the text from the file into memory.
    lseek(fd, li.id_faddr, 0);
    segment_size = li.id_npg << PAGESHIFT;

    if (read(fd, (void *) li.id_vaddr, segment_size) != segment_size) {
        close(fd);
        return KILL;
    }



    // Now set the page table entries for the program text to be readable
    // and executable, but not writable.
    for (i=0; i<li.t_npg; i++) {
        PTE old_entry = process->page_table->entries[text_pg1 + i];
        PTE new_entry = createPTEWithOptions(text_options, (long)pageAtIndex(old_entry.pfn));
        process->page_table->entries[text_pg1 + i] = new_entry;
    }

    // Flush the TLB for region 1 now that we've updated the PTEs for the text
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);




// ==>> Change the protection on the "li.t_npg" pages starting at
// ==>> virtual address VMEM_1_BASE + (text_pg1 << PAGESHIFT).    Note
// ==>> that these pages will have indices starting at text_pg1 in 
// ==>> the page table for region 1.
// ==>> The new protection should be (PROT_READ | PROT_EXEC).
// ==>> If any of these page table entries is also in the TLB, either
// ==>> invalidate their entries in the TLB or write the updated entries
// ==>> into the TLB.    It's nice for the TLB and the page tables to remain
// ==>> consistent.

    close(fd);			/* we've read it all now */

    /*
     * Zero out the uninitialized data area
     */
    memset(&(li.id_end), 0x00, li.ud_end - li.id_end);

    /*
     * Set the entry point in the exception frame.
     */
// ==>> Here you should put your data structure (PCB or process)
// ==>>    proc->context.pc = (caddr_t) li.entry;




    
    // Now, finally, build the argument list on the new stack.
    #ifdef LINUX
        memset(cpp, 0x00, VMEM_1_LIMIT - ((long) cpp));
    #endif


    *cpp++ = (char *) argcount;		    // the first value at cpp is argc
    cp2 = argbuf;
    
    for (i=0; i < argcount; i++) {    // copy each argument and set argv
        *cpp++ = cp;
        strcpy(cp, cp2);
        cp += strlen(cp) + 1;
        cp2 += strlen(cp2) + 1;
    }

    free(argbuf);
    *cpp++ = NULL;			/* the last argv is a NULL pointer */
    *cpp++ = NULL;			/* a NULL pointer for an empty envp */

    return SUCCESS;
}
