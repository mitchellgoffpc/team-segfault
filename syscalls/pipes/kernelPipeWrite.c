//PipeWrite
/*
Method: write byte to a pipe buffer

params: pipe_id (pipe to be written to),
		buf (contians the content that we will be writing to the pipe),
		len (number of bytes to write to the pipe)

returns: number of bytes written
*/

void kernelPipeWrite(UserContext *) {
	int pipe_id = UserContext.regs[0];
	void *buf = UserContext.regs[1];
	int len = UserContext.regs[2];

	//while there is still stuff from buf to write to pipe
		//write stuff from buf to pipe
	//regs[0] = len (number of bytes written) when complete, ERROR else
}