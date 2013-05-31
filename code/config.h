/*
 * Copyright (c) 2012 David Brandt
 *  All rights reserved.
 *
 * Licensed under: CC BY-NC-SA 3.0 (full license in file LICENSE)
 * 
 * You are free:
 * - to Share - to copy, distribute and transmit the work
 * - to Remix - to adapt the work
 * 
 * Under the following conditions:
 * - Attribution - You must attribute the work in the manner specified by
 *   the author or licensor (but not in any way that suggests that
 *   they endorse you or your use of the work).
 * 
 * - Noncommercial - You may not use this work for commercial purposes.
 * 
 * - Share Alike - If you alter, transform, or build upon this work, you may
 *   distribute the resulting work only under the same or similar license
 *   to this one.
 */

#ifndef	CONFIG_H
#define	CONFIG_H
#define NUMSLAVES			7
#define NUMCELLS			84
#define NUMTEMPS			42

// these must be set as in the canlib
#define	SUPPORT_EXTENDED_CANID	1
#define	SUPPORT_TIMESTAMPS		0

// slave pins
#define SLAVECS				A,7
#define SLAVESDO			B,3

// relai pins
#define RELAIPLUS_CTRL				E,2
#define RELAIPLUS					B,5
// AND output of RELAI_FF+, RELAIPDCLOSED and RELAID-CLOSED
#define RELAIPLUS_AND				E,7
// inverse of the above
#define RELAIPLUS_AND_INV			C,1
// relai+ signal after the delaying ne555
#define RELAIPLUS_DELAYED			E,5
// fet controlling output
#define RELAIPLUS_OUT				E,6

#define RELAIMINUS_CTRL				A,4
#define RELAIMINUS					E,4

#define RELAICHARGE_CTRL			E,0
#define RELAICHARGE					E,3

#define RELAI_FFCLK					E,1

#define RELAI_VCC					A,0

// adc spi
#define ADCCS						C,2

// sd card cs
#define SDCS						A,2

// LEDS
#define LED_R						B,7
#define LED_G						G,3
#define LED_B						G,4

// precharge current and 40v lines
#define N40V						B,6
#define NPRECHC						B,4

#define GPOC1						D,0
#define GPOC2						D,1
#define DCDCCTRL					D,1

#define MV_TO_16BIT(x)					(((uint32_t)x)*65536/409600)

// outgoing message ids
#define CAN_ERROR_ID			0x2E0
#define CAN_RESET_ERROR_ID		0x2E1
#define CAN_STATUS_ID			0x2EA
#define CAN_DEBUG_ID			0x2EF
#define CAN_VOLTAGE_WARN		0x2F0
#define CAN_PRECHARGE_DONE_ID	0x2F3
#define CAN_STATE_ID			0x300
#define CAN_BASE_CELL_ID		0x500
#define CAN_FULL_FSM_ID			0x6A0
#define CAN_OUTPUT_ADC			0x6A1
#define CAN_CELL_MIN_MAX_AVG	0x4FF
#define CAN_TEMP_MIN_MAX_AVG	0x4FE
#define CAN_MEASURE_CYCLE_MS	0x6C0
#define CAN_LV_MEASURE			0x301

// incoming command ids
#define CAN_TRACTIVE_ENABLE		0x2F0
#define CAN_TRACTIVE_DISABLE		0x2F1
#define CAN_BALANCING_ENABLE		0x2F2
#define CAN_BALANCING_DISABLE		0x2F3
#define CAN_CHARGE_ENABLE			0x2FA
#define CAN_CHARGE_DISABLE			0x2FB
#define CAN_DSPACE_DC_LINK_VOLTAGE	0x310

// dpsace MUSt send this often (in ms) the hartbeat
#define DSPACE_HEARTBEAT_MS		500

// intermediate settings

// maximum time a precharge to charge mode should take
#define PRECHARGE_CHARGE_CC			0
#define PRECHARGE_CHARGE_MC			500
#define PRECHARGE_CHARGE_PC			1000
#define PRECHARGE_CHARGE_MAX			2000

// maximum and minimum times a precharge to tractive mode should take
// times in ms when to close the relais
#define PRECHARGE_TRACTIVE_CC			0
#define PRECHARGE_TRACTIVE_MC			500
#define PRECHARGE_TRACTIVE_PC			4000
// defined time where the precharge MUST be active to check broken cables
#define PRECHARGE_TRACTIVE_PRECHARGE	1500
// minimum time needed, to check for broken parts/cables
#define PRECHARGE_TRACTIVE_MINTIME		2500
// maximum time the precharge should take
#define PRECHARGE_TRACTIVE_MAX			5000

// under/overvoltage treshhold
// formula: x = (Voltage / 1.5) + 512
// Voltage in mV

#define SLAVE_VOLTAGE_MAX				3332 // 4.230V
#define SLAVE_VOLTAGE_BALANCE			3312 // 4.200V
//#define SLAVE_VOLTAGE_BALANCE	3045 // 3.800V
#define SLAVE_VOLTAGE_WARN				2345 // 2.750V
#define SLAVE_VOLTAGE_WARN_TIME		10
#define SLAVE_VOLTAGE_MIN				2312 // 2.700V

// upper limit: 55°C -> (55°C+40°C) * 2 = 190
#define SLAVE_TEMPERATURE_MAX			190
#define SLAVE_TEMPERATURE_WARN			186
#define SLAVE_TEMPERATURE_WARN_TIME	10

// maximum grace time for over and under voltage in milliseconds
#define SLAVE_UV_TIME			1000
#define SLAVE_OV_TIME			300
#define SLAVE_OT_TIME			1000

#define SLOWDOWN_VOLTAGE		3172 // 3.99V
#define IMMEDIATE_SHUTDOWN_OV	3198 // 3.93V
// 4.23V -> 3332

// maximum time a complete measuring cycle on the slaves should take
#define SLAVE_CYCLE_MAX_MS			2000
#define SLAVE_CYCLE_WARN_MS		1500
#define SLAVE_STATUS_CYCLE_LENGTH	100
#define SLAVE_MAX_ERRORS			1000
#define SLAVE_CYCLE_WARN_TIME		2

#define SLAVE_WAIT_CELL_MEASURE	20
#define SLAVE_WAIT_TEMP_MEASURE	10
#define SLAVE_WAIT_DIAG				20

// WDT = 1, GPIO2 = 0, GPIO1 = 0, LVLPL = 1, CELL10 = 0, CDC2 = 0, CDC1 = 0, CDC0 = 1
#define SLAVE_DEFAULT_CONFIG		0x91

// BALANCING
// balancing should be done with > 3.9V
#define BALANCE_MIN_CELL_VOLT	3112
// this defines the factor used to determine the time to balance according to the voltage difference
// it is just an estimate current, based off the following:
// the capacity from 4.2V to 3.8V is relatively precise 9Ah
// 9000mAh / 400mV = 22,5 mAh/mV = 81000mAs/mV
// we have a balancing resistor of 10Ohm, which can only be turned on 1/3 of the time
// so 4,2V / 3 * 10Ohm = 140mA current is assumed
// which leads us to 81000mAs/mv / 140mA -> 579 s/mV
// we don't want to do all the discharge in one cycle, i assume 2-4 cycles, using 450 s/mV
// this way we can use a 16 bit integer for the seconds
#define BALANCE_TIME_FACTOR	500

// the time that should be waited until the next balance cycle is started
#define BALANCE_WAIT_CYCLE		900

// minimum difference between cells to start balancing
#define BALANCE_MIN_DIFF		3

#define ADC_WAIT_TIME			1

#define VCC_MIN					95
#define VCC_RELAI_MIN			90

#endif	// CONFIG_H
