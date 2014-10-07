//Wait

/*
Method: collect process ID and exit status returned by child
		process of calling program; exit status added to 
		FIFO queue (info removed from queue after wait returns)

params: status_ptr (where to save the child's exit status)

returns: process ID of child
*/

int wait(int* status_ptr) {
	//grab process ID
	//grab exit status of child
	//save child's exit status in status_ptr

	//put child's exit status in FIFO queue
	//remove the child's info from the FIFO queue once wait is finished

	//block the calling process until the child returns

	//return process ID of child process, ERROR upon failure

}