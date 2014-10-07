//SetKernelData
/*
Method: gives you values you need to determine pages that were 
initially used by kernel at boot time

params: kernelDataEnd (lowest address not in use by kernel's
		instructions or global variables), kernelDataStart
		(lowest address used by data segment)

returns: nothing; just sets the data
*/

void setKernelData(void* kernelDataStart, void * kernelDataEnd) {
	//
}
