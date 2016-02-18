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

#ifndef RELAI_H
#define	RELAI_H

#define SET_RELAI(r)		SET2(r); RESET(RELAI_FFCLK); _delay_us(5); SET(RELAI_FFCLK)
#define RESET_RELAI(r)		RESET2(r); RESET(RELAI_FFCLK); _delay_us(5); SET(RELAI_FFCLK)

void relai_init(void);#endif	// RELAI_ _
