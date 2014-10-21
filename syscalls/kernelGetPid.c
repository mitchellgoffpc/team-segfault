/*
	File: kernelGetPid
	Date: 10/20/2014
	Author: Emily Holt and Mitchell Goff
/*

Method: return process ID
*/

void kernelGetPid(UserContext *) {
	UserContext.regs[0] = ((ProcessInfo *) KERNEL_STACK_BASE)->current_brk;

}