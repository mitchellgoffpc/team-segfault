//Exec method

/*
Method: replaces currently running process with program stored in filename
params: filename (the name of the program to start running) and all parameters
		the new program needs
returns: nothing because the old process was erased?
*/
void kernelExec(UserContext *) {
	char* filename = UserContext.regs[0];
	char** argvec = UserContext.regs[1];
	//copy arguments from user space into kernel buffer
	//open the executable
	//create a new address space
	//load executable into it
	//copy arguments from kernel buffer into user stack
	//return to user mode running the new process
}