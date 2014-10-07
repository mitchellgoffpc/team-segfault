//Brk
/* 
Method: sets the lowest location not used by the program
*/

void kernelBrk(UserContext *) {
	void *addr = UserContext.regs[0];
	//need some variable to keep track of lowest location not used by program
	//modify addr to be rounded up to the next multiple of PAGESIZE bytes
	//set that variable to be equal to addr
	//return ERROR if one occurs
}