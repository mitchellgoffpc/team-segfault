//KernelCvarBroadcast

/*
Method: broadcast condition variable

params: cvar_id (condition variable to broadcast to)
*/

void kernelCvarBroadcast(UserContext *) {
	int cvar_id = UserContext.regs[0];

	//go through all waiters
	//wake them up
	//put them all on the ready queue
}