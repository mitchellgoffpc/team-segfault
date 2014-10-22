/*
	File: getPid.c
	Date: 10/20/2014
	Author: Emily Holt and Mitchell Goff
/*

/*
Method: return process ID
*/

void KernelGetPid(UserContext *context) {
	context.regs[0] = ((ProcessInfo *) KERNEL_STACK_BASE)->pid;

}