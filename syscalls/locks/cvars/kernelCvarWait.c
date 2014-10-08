//KernelCvarWait
/*
Method: release the lock and wait on condition variable
		after waking up, re-acquire the lock

params: cvar_id (the condition variable to wait on),
		lock_id (the lock to release)
*/

void kernelCvarWait(UserContext *) {
	int cvar_id = UserContext.regs[0];
	int lock_id = UserContext.regs[1];

	//release the lock

	//while (condition is not true)
		//wait (suspaned all operations)

	//re-acquire the lock

	//return ERROR if necessary

}