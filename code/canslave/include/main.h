#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>
#include "type.h"
#include "config.h"

#include "math.h"
#include "LPC11xx.h"
#include "crp.h"

#include "adc.h"
#include "gpio.h"
#include "ssp.h"
#include "slave.h"
#include "can.h"
#include "iap.h"
#include "ntc.h"

extern volatile uint32_t SystemCoreClock;

void setSlaveState(uint8_t err);

enum
{
	SLAVE_ERR = 0x1,
	SLAVE_COMMERR = 0x02,
	SLAVE_OVERTEMP = 0x4,
	SLAVE_VOLTAGEERR = 0x8,
	SLAVE_IC_FAIL = 0x10,
	SLAVE_WIRE_BROKEN = 0x20,
	SLAVE_BALANCING = 0x40,
	SLAVE_POWERR = 0x80
};

extern volatile uint8_t slaveStatus;
extern volatile uint8_t slaveID;

#endif
