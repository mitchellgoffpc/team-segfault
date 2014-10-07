//KernelCvarInit

/*
Method: create a new condition variable
 		and save its identifier

params: cvar_idp (where the the new cvar's identifier will be stored)
*/

void kernelCvarInit(UserContext *) {
	int *cvar_idp = UserContext.regs[0];
	//create a new condition variable
	//cvar_idp = identifier of new condition variable
}