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

#ifndef SPI_H
#define	SPI_Henum {	SPI_TRANSFERING = 0,
	SPI_IDLE,
	SPI_DONE,	SPI_POLLING,
	SPI_DISABLE
};#define spi_start_transfer() spi_buffer_pos = spi_status = 0; SPDR = spi_buffer[0]#define spi_set_idle() spi_status = SPI_IDLE#define spi_transfering()		(spi_status == SPI_TANSFERING)#define spi_done()				(spi_status == SPI_DONE)#define spi_idle()				(spi_status == SPI_IDLE)extern volatile unsigned char spi_buffer[256], spi_buffer_pos, spi_buffer_size, spi_status;void spi_init(void);uint8_t spi_put(uint8_t spi_data);void spi_print_to_write(void);#endif	// SPI_H
