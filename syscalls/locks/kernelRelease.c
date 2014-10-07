//KernelRelease

/*
Method: release the lock identified by lock_id

params: lock_id (lock to be released)
*/

void kernelRelease(UserContext *) {
	int lock_id = UserContext.regs[0];
	//if user currently holds the lock
		//release it
	//else
		//ERROR
}