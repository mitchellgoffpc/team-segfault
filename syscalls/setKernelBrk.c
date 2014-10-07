//setKernelBrk
/*
Method: indicates the lowest address location not used
		(not yet needed by malloc)

params: addr (pointer to the address which will be the
		new lowest location not used by kernel)

returns: 0 upon success, ERROR if failure
*/

int setKernelBrk(void* addr) {
	//need access to kernelBrk variable
	//set that global variable to addr
}