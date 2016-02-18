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

#include "config.h"
#include "main.h"
#include "spi.h"#include "spi_adc.h"

volatile uint32_t adc_volt = 0;
volatile uint32_t adc_in = 0;

void spi_adc_init(void)
{
	SET_OUTPUT(ADCCS);
	SET(ADCCS);}

void spi_adc_start(void)
{
	RESET(ADCCS);
	spi_buffer_size = 2;
	spi_start_transfer();
}

void spi_adc_result(void)
{
	SET(ADCCS);
	spi_status = SPI_IDLE;
	adc_in = ((spi_buffer[0] << 8) & 0xFF00) | (spi_buffer[1] & 0xFF);
}

