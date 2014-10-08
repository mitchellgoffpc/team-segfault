//Trap_Tty_Receive

/*
Method: new line of input is available from the terminal

params: code field in UserContext
*/

void trapTtyReceive(UserContext *) {
	//get the correct terminal ID from UserContext.code;
	//use TtyReceive to grab the input in that terminal
	//buffer the input line for TtyRead call
}