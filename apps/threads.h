/*
  File: threads.h
  Date: 10/6/2014
  Author: Mitchell Goff
*/

#ifndef __USER_THREADS_H__
#define __USER_THREADS_H__



/* =============================== *

  			  Includes

 * =============================== */

#include "../include/hardware.h"
#include "../include/yalnix.h"





/* =============================== *

  		   Data Structures

 * =============================== */

typedef void (*ThreadEntry) (void *);





/* =============================== *

  		     Interface

 * =============================== */

int CreateThread(ThreadEntry entry, void *entry_arg);
int JoinThread(int thread_id);



#endif

