/*
  File: traps.h
  Date: 10/6/2014
  Author: Emily Holt & Mitchell Goff
*/

#ifndef __YALNIX_TRAPS_H__
#define __YALNIX_TRAPS_H__



/* =============================== *

  	          Includes

 * =============================== */

#include "../include/hardware.h"
#include "../sync/sync.h"





/* =============================== *

  	           Data

 * =============================== */

struct TTY;
typedef struct TTY TTY;

struct TTY {
	WaitQueue write_queue;
	WaitQueue read_queue;

	PID write_current;
	void *read_buffer;
	long read_buffer_size;
	long read_buffer_position;
};


extern TTY ttys[NUM_TERMINALS];
extern long elapsed_clock_ticks;





/* =============================== *

  	           Macros

 * =============================== */

#define register(x) \
    getCurrentProcess()->user_context.regs[x]




/* =============================== *

  	          Interface

 * =============================== */

void trapClock(UserContext *context);
void trapDisk(UserContext *context);
void trapTtyReceive(UserContext *context);
void trapTtyTransmit(UserContext *context);

void trapKernel(UserContext *context);

void trapIllegal(UserContext *context);
void trapMemory(UserContext *context);
void trapMath(UserContext *context);


int ttyRead(int tty, void *u_buffer, int u_length);
void ttyReadBegin(int tty);

int ttyWrite(int tty, void *u_buffer, int length);
void ttyWriteFinished(int tty);



#endif
