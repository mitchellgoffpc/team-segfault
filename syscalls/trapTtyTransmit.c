//Trap_Tty_Transmit

/*
Method: signal the TtyTransmit hardware operation is complete

params: code field in UserContext (indicated the specific terminal)
*/

void trapTtyTransmit(UserContext *context) {
	//get the specific terminal from UserContext.code
	//complete the blocked process that started this terminal output
	//start next terminal output
}