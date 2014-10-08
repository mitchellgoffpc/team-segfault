//KernelCvarSignal

/*
Method: signel the condition variable using mesa-style semantics

params: cvar_id (condition variable to signal)
*/

void kernelCvarSignal(UserContext *) {
	int cvar_id = UserContext.regs[0];

	
	//notify the waiter that condition is now true
	//put that waiter on the ready queue
}