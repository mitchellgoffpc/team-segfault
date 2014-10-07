//exit
/*
Method: terminate a process, save the status value 
		somewhere important; free all other resources
		used by this process; don't disturb the children

params: the status to be saved

returns: absolutely nothing
*/

void exit(int status) {
	//save the status value somewhere important, 
	//in a place where the value won't get deleted

	//go through all resources (specified in PCB?)
	//and free them all

	//go through all children and make sure 
	//they no longer point back to the parent?
}