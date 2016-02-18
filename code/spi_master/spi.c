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
#include <avr/interrupt.h>#include "spi.h"
#include "config.h"
#include "main.h"
#include "mcan.h"
#include <util/delay.h>
volatile unsigned char spi_buffer[256], spi_buffer_pos = 0, spi_buffer_size = 0, spi_status = 0;void spi_init(void)
{
	// set MOSI, SCK and SS as output
	DDRB |= 0x06;
	SPCR =  (1 << MSTR) | /* Master mode */
			(1 << SPE)  | /* SPI Enable */
			(1 << SPIE)  | /* SPI Interrupt Enable */
			(0 << DORD) | /* Data Order: MSB first */
			(1 << CPOL) | /* Clock Polarity: SCK High when idle */
			(1 << CPHA) | /* Clock Phase: sample on rising SCK edge */
			(1 << SPR0) ; /* Clock Frequency: f_OSC / 16 = 1Mhz */
	
	spi_status = SPI_IDLE;
	
	SET_OUTPUT2(B,0);
	SET2(B,0);
	
	SET_OUTPUT(SDCS);
	SET(SDCS);
}

// this is executed after every transfered spi byte
ISR( SPI_STC_vect ){
	char cSREG;
	cSREG = SREG;
	cli();
	if( spi_status == SPI_TRANSFERING )
	{
		spi_buffer[spi_buffer_pos++] = SPDR;
		if( spi_buffer_pos == spi_buffer_size )
		{
			spi_status = SPI_DONE;
		}
		else
		{
			SPDR = spi_buffer[spi_buffer_pos];
		}
	}
	sei();
	SREG = cSREG;
}uint8_t spi_put(uint8_t spi_data)
{
	SPDR = spi_data;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}
