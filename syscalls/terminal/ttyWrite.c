//TtyWrite
/*
Method: write contents of buffer to the terminal

params: tty_id (terminal to write to),
		buf (content that will get written to terminal)
		len (length of the buffer)

returns: number of bytes written on success, ERROR else
*/

int ttyWrite(int tty_id, void *buf, int len){
	//while we are not finished writing
		//block the calling process

	//if len < TERMINAL_MAX_LINE
		//write the contents of buffer to terminal output

	//else
		//while current_index_in_buffer < len
			//write next TERMINAL_MAX_LINE chunks to the terminal
			//increment current_index_in_buffer by TERMINAL_MAX_LINE
}