//Delay
/*
Method: delay the calling process for the given number of clock ticks
*/
void KernelDelay(UserContext *) {
	int clock_ticks;
	clock_ticks = UserContext.regs[0];

	//if clock_ticks is 0 return immediately
	if (clock_ticks == 0) return 0;

	//if clock_ticks < 0 don't time travel, return an Error
	if (clock_ticks < 0) {
		TracePrintf(1, "Hey! We can't go back in time!\n");
		return -1;
	}

	//block the calling process until clock_ticks clock interrupts have occurred
	int counter;
	counter = 0;
	while (counter < clock_ticks) {

		counter++;
	}
	

}