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





/* =============================== *

  	           Data

 * =============================== */

extern long elapsed_clock_ticks;





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



#endif