//Delay
/*
Method: delay the calling process for the given number of clock ticks
*/
void kernelDelay(UserContext *) {
	int clock_ticks = UserContext.regs[0];
	//if clock_ticks = 0
	//return immediately

	//if clock_ticks < 0
	//return ERROR

	//int counter
	//counter = 0
	//while counter <= clock_ticks
	//block the calling process until clock_ticks clock interrupts have occurred

}