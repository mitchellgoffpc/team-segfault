//Fork method
/*Method: creates a new process by copying all of the parent process's 
page tables into the new address space w/new process ID

params: none

returns: 0 if child process was created successfully, ERROR if not
*/

int fork(void) {
	//save user context from when the user executed TRAP_KERNEL to call fork
	//generate new process ID
	//allocate space for new PCB
	//create kernel context for child process (copied from parent's kernel context)
	//allocate space for child process
	//copy parent's memory space contents
	//return 0 upon success
}
