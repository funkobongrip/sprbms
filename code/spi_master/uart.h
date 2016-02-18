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

#ifndef UART_H
#define	UART_H

extern volatile unsigned char uart_out_buffer[32], uart_out_buffer_size, uart_out_buffer_pos, uart_out_status;
extern volatile unsigned char uart_in_buffer[32], uart_in_buffer_size, uart_in_buffer_pos, uart_in_buffer_ovf, uart_in_status;

#define uart_idle() (uart_out_status == 0)
#define uart_received() (uart_in_status == 1)
#define uart_start_send() uart_out_buffer_pos = uart_out_status = 0; UDR0 = uart_out_buffer[0]; UCSR0B |= (1<<UDRIE0)
#define uart_start_recv(size) uart_in_buffer_pos = uart_in_status = uart_in_buffer_ovf = 0; uart_in_buffer_size = size
#define uart_send_done() (uart_out_status == 1)
#define uart_recv_done() (uart_in_status == 1)

#define uart_send_str(str) strcpy((char *)uart_out_buffer, str); uart_out_buffer_size = strlen((char*)uart_out_buffer); uart_start_send()

void uart_init(void);
int uart_putchar(char c, FILE *stream);
void uart_str(char *out);
void uart_binary(unsigned char out);

#endif	// UART_H
