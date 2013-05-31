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
#include <avr/interrupt.h>#include "uart.h"//static FILE uartstdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE); 

volatile unsigned char uart_out_buffer[32], uart_out_buffer_size, uart_out_buffer_pos, uart_out_status;
volatile unsigned char uart_in_buffer[32], uart_in_buffer_size, uart_in_buffer_pos, uart_in_buffer_ovf, uart_in_status;
void uart_init(void)
{
	UBRR0H = 0; // 9600 baud, 0.2% error
	UBRR0L = 103;
	
	uart_in_status = 0;
	uart_out_status = 0;

	// Aktivieren von receiver und transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);

	// Einstellen des Datenformats: 8 Datenbits, 1 Stoppbit
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);	//stdout = &uartstdout;
}

int uart_putchar(char c, FILE *stream)
{
	while (!(UCSR0A & (1<<UDRE0))) {}
	UDR0 = c;
	return 0;
}

void uart_str(char *out)
{
	char *tmp;
	tmp = out;
	while( *tmp != 0 )
	{
		uart_putchar(*(tmp++), NULL);
	}
}void uart_binary(unsigned char out)
{
#define PUTBINARY(x,y)  uart_putchar( ((x&y)==y)?'1':'0', NULL);
	PUTBINARY(out, 0x80);
	PUTBINARY(out, 0x40);
	PUTBINARY(out, 0x20);
	PUTBINARY(out, 0x10);
	PUTBINARY(out, 0x08);
	PUTBINARY(out, 0x04);
	PUTBINARY(out, 0x02);
	PUTBINARY(out, 0x01);
}

ISR( USART0_RX_vect ){
	if( uart_in_buffer_pos > uart_in_buffer_size )
	{
		uart_in_buffer_ovf = UDR0;
	}
	else
	{
		uart_in_buffer[uart_in_buffer_pos++] = UDR0;
	}

	if( uart_in_buffer_pos == uart_in_buffer_size )
	{
		uart_in_status = 1;
	}
}

ISR( USART0_UDRE_vect ){
	if( uart_out_buffer_pos == uart_out_buffer_size )
	{
		uart_out_status = 1;
		UCSR0B &= ~(1<<UDRIE0);
	}
	else
		UDR0 = uart_out_buffer[++uart_out_buffer_pos];
}

