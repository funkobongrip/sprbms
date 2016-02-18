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

#ifndef MAIN_H
#define	MAIN_H

extern volatile unsigned char battery_state;
extern volatile unsigned char slave_state;
extern volatile unsigned char relai_state;
extern volatile unsigned char relai_ext_state;
// warning time
extern volatile unsigned char warning_time_val;

volatile char error_msg[9];

void led_blink_halt(void);
void error(char *msg);
void status(char *msg);
void standby(void);
void update_states(void);
void warning_time(unsigned char time);
// ----------------------------------------------------------------------------------------------------------------------------- //// Status message, sent every X ms, no ACK means BMS will shut off.// CAN: <Status 8 Bit><Ext Status 8 Bit><Error Code 8 Bit><DC Bus Voltage 16 Bit><Outputs Status 8 Bit><Inputs Status 8 Bit>// Status Bits:enum {	STATE_ERROR = 0x01, // an error occured and can be tried reset with a reset frame
	STATE_WARNING = 0x02,	/*STATE_PRECHARGE = 0x04, // precharge
	STATE_DISCHARGE = 0x08, // discharge*/
	STATE_INTERMEDIATE = 0x04,	STATE_TRACTIVE = 0x10,  // hv relais closed	STATE_CHARGING = 0x20, // charging	STATE_BALANCING = 0x40 // balancing on};

enum {
	RELAI_PD_EN = 0x01,
	RELAI_PD_CL = 0x02,
	RELAI_M_EN = 0x04,
	RELAI_M_CL = 0x08,
	RELAI_P_EN = 0x10,
	RELAI_P_CL = 0x20,
	RELAI_40V = 0x40,
	RELAI_PRECH = 0x80
};

enum {
	RELAI_EXT_AND = 0x01,
	RELAI_EXT_AND_INV = 0x02,
	RELAI_EXT_DELAYED = 0x04,
	RELAI_EXT_OUT = 0x08,
	RELAI_EXT_VCC = 0x80
};

#define RESET_LEDS() RESET_LED(LED_R); RESET_LED(LED_G); RESET_LED(LED_B)
#define SET_LED(l)		SET2(l)
#define RESET_LED(l)	RESET2(l)// Output Status Bits: <RELAI-><RELAIPD><RELAI+><FAN><BUZ><GPIO><BATEN><DISBUZ>// Input Status Bits: <RESERVED><RESERVED><RESERVED><RESERVED><B40V><PRECH_C><RELAIVCC_OK><VCC_OK>// ----------------------------------------------------------------------------------------------------------------------------- //// ----------------------------------------------------------------------------------------------------------------------------- //// CELL message sent every X ms// CAN: <CellV N 12 Bit><CellV N+1 12 Bit><Cell N+2 12 Bit><Cell N+3 12 Bit><Temp N 8Bit><Temp N+2 8 Bit>// Cell Voltage unsigned 12 bit integer in mV// Temperature in unsigned 8 bit integer in 0.5K starting at 273,15K// ----------------------------------------------------------------------------------------------------------------------------- //// ----------------------------------------------------------------------------------------------------------------------------- //// LV Voltage message, sent every X seconds// CAN: <VCC Voltage 10 Bit><RELAI Voltage 10 Bit>// Voltage is 0-20V in 20V/1024 Steps// ----------------------------------------------------------------------------------------------------------------------------- //// ----------------------------------------------------------------------------------------------------------------------------- //// Error message, sent upon error occuring// CAN: <Error Code 8 Bit><Error Description 56 Bit>// Error Code as in Status Message, Description is a 8 character long 7 bit String Describing the problem// e.g.// OV C 100// OT C 012// UV C 123// VCC LOW// RVCC LOW// SLERR 10// ----------------------------------------------------------------------------------------------------------------------------- //// ----------------------------------------------------------------------------------------------------------------------------- //// Debug Message// CAN <Stuff 64 Bit>// ----------------------------------------------------------------------------------------------------------------------------- //
// 
#define	PORT(x)			_port2(x)
#define	DDR(x)			_ddr2(x)
#define	PIN(x)			_pin2(x)
#define	REG(x)			_reg(x)
#define	PIN_NUM(x)		_pin_num(x)

#define	RESET(x)		RESET2(x)
#define	SET(x)			SET2(x)
#define	TOGGLE(x)		TOGGLE2(x)
#define	SET_OUTPUT(x)	SET_OUTPUT2(x)
#define	SET_INPUT(x)	SET_INPUT2(x)
#define	SET_PULLUP(x)	SET2(x)
#define	IS_SET(x)		IS_SET2(x)

#define	SET_INPUT_WITH_PULLUP(x)	SET_INPUT_WITH_PULLUP2(x)

#define	_port2(x)	PORT ## x
#define	_ddr2(x)	DDR ## x
#define	_pin2(x)	PIN ## x

#define	_reg(x,y)		x
#define	_pin_num(x,y)	y

#define	RESET2(x,y)		PORT(x) &= ~(1<<y)
#define	SET2(x,y)		PORT(x) |= (1<<y)
#define	TOGGLE2(x,y)	PORT(x) ^= (1<<y)

#define	SET_OUTPUT2(x,y)	DDR(x) |= (1<<y)
#define	SET_INPUT2(x,y)		DDR(x) &= ~(1<<y)
#define	SET_INPUT_WITH_PULLUP2(x,y)	SET_INPUT2(x,y);SET2(x,y)

#define	IS_SET2(x,y)	((PIN(x) & (1<<y)) != 0)
// #endif	// MAIN_H
