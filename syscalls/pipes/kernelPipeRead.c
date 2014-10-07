//PipeRead

/*
Method: read bytes from the pipe into the buffer

params: pipe_id (pipe to read from),
		buf (where the read bytes are going to go),
		len (how many bytes we are going to read)

returns: number of bytes read 
*/

void kernelPipeRead(UserContext *) {
	int pipe_id = UserContext.regs[0];
	void *buf = UserContext.regs[1];
	int len = UserContext.regs[2];
	//while bytes are not currently available to read
		//block the calling process

	//read len bytes into buf

	//regs[0] = len on success, ERROR else
}