//KernelCvarSignal

/*
Method: signel the condition variable using mesa-style semantics

params: cvar_id (condition variable to signal)
*/

void kernelCvarSignal(UserContext *) {
	int cvar_id = UserContext.regs[0];

	
	//while the condition is not true
		//wait
	//now that the condition is true
	//signal the condition variable at cvar_id
}