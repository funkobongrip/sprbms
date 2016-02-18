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

#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "main.h"
#include "relai.h"

void relai_init(void)
{
	RESET(RELAIPLUS_CTRL);
	RESET(RELAIMINUS_CTRL);
	RESET(RELAICHARGE_CTRL);
	SET_OUTPUT(RELAIPLUS_CTRL);
	SET_OUTPUT(RELAIMINUS_CTRL);
	SET_OUTPUT(RELAICHARGE_CTRL);
	SET_OUTPUT(RELAI_FFCLK);
	
	RESET(RELAI_FFCLK);
	_delay_us(5);
	SET(RELAI_FFCLK);
	_delay_us(5);
	RESET(RELAI_FFCLK);
	
	RESET(RELAIPLUS);
	RESET(RELAIMINUS);
	RESET(RELAICHARGE);
	SET_INPUT(RELAIPLUS);
	SET_INPUT(RELAIMINUS);
	SET_INPUT(RELAICHARGE);
}
