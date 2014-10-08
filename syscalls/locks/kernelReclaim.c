//KernelReclaim
/*
Method: destroy the lock, pipe, or condition variable

params: id (identifier of object to be destroyed)
*/

void kernelReclaim(UserContext *) {
	int id = UserContext.regs[0];

	//free the memory of the object
	//make sure nothing points back to it
	//release all associated resources
}