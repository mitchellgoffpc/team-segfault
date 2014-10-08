//Trap_Kernel
/*
Method: execute the requested kernel call

params: kernal call number (in teh code field of UserContext)

returns: (in the regs[0] of UserContext)
*/

void trapKernel(UserContext * ){
	int call_number = UserContext.regs[0];

	//load the requested kernel call
	//execute the requested kernel call
	//save the return value from the kernel call in regs[0]

	//UserContext.regs[0] = return val
}