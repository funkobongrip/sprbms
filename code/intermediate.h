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

#ifndef INTERMEDIATE_H
#define	INTERMEDIATE_H

extern volatile uint16_t precharge_time;

#define RELAI_STATE_PRECHARGING	(RELAI_PD_EN | RELAI_PD_CL | RELAI_M_EN | RELAI_M_CL | RELAI_40V)
#define RELAI_STATE_PRECHARGE_DONE	(RELAI_PD_EN | RELAI_PD_CL | RELAI_M_EN | RELAI_M_CL | RELAI_40V | RELAI_PRECH)
#define RELAI_STATE_READY			(RELAI_PD_EN | RELAI_PD_CL | RELAI_M_EN | RELAI_M_CL | RELAI_P_EN | RELAI_P_CL | RELAI_40V | RELAI_PRECH)

void intermediate_init(void);
unsigned int intermediate_loop(void);

#endif // INTERMEDIATE_H
