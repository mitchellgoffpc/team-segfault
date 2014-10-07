//PipeInit
/*
Method: create a new pipe

params: pipe_idp (pointer to where the new pipe identifier will go)

returns: ERROR upon failure
*/

void kernelPipeInit(UserContex *) {
	int *pipe_idp = UserContex.regs[0]
	//create a new pipe

	//pipe_idp = new pipe's identifier
}
