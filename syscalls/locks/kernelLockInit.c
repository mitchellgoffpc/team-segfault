//KernelLockInit
/*
Method: create a new lock

params: lock_idp (where the lock identifier will be stored)

returns: 0 on success, ERROR if it happens

*/

void kernelLockInit(UserContext *) {
	int *lock_idp = UserContex.regs[0];
	//create a new lock
	//lock_idp = the new lock's identifier
	//if there's an error, return it
}