//Trap_Memory

/*
Method: enlarge the process's stack to cover the requested address
		if its possible, else kill the current process

params: addr (address that the stack should now cover)

*/

void trapMemory(UserContext *) {
	int addr = UserContext.addr;

	//grow the stack
	//and continue running the process
	
	//else kill the process
}