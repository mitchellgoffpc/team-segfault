//TtyRead
/*
Method: Read next line of input from terminal and copy it into a buffer

params: tty_id (terminal id to copy from), 
		buf (buffer where input is going to be stored),
		len (maximum length of line to be copied)

returns: the number of bytes actually copied on success, ERROR else
*/

int ttyRead(int tty_id, void *buf, int len){
	//while theres no input to be returned
	//block the calling process

	//grab the terminal associated with tty_id

	//if the size of that input is <= len bytes
	//copy the input from that terminal 
	//(only copy that input, not the full len bytes if there's less that len bytes)

	//else 
	//call ttyRead on input for next set of len bytes

	//return number of bytes actually copies


}