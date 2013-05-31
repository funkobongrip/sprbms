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

#ifndef SLAVE_H
#define	SLAVE_Henum {// write config	SLAVE_CMD_WRITE_CONFIG = 0x01,// read config
	SLAVE_CMD_READ_CONFIG = 0x02,// verify config (no ltc6803 command!)	SLAVE_CMD_VERIFY_CONFIG = 0x03,// read all voltages
	SLAVE_CMD_READ_VOLTAGES = 0x04,// read voltages for cells 1-4	SLAVE_CMD_READ_VOLTAGES14 = 0x06,// read voltages for cells 5-8	SLAVE_CMD_READ_VOLTAGES58 = 0x08,// read voltages for cells 9-12	SLAVE_CMD_READ_VOTLAGES912 = 0x0A,// read cell flags (UV/OV/OT)	SLAVE_CMD_READ_FLAGS = 0x0C,// read temperatures (die)	SLAVE_CMD_READ_TEMP = 0x0E,// start cell conversion for all or specific	SLAVE_CMD_CONV_CELLS = 0x10,	SLAVE_CMD_CONV_CELL1,	SLAVE_CMD_CONV_CELL2,	SLAVE_CMD_CONV_CELL3,	SLAVE_CMD_CONV_CELL4,	SLAVE_CMD_CONV_CELL5,	SLAVE_CMD_CONV_CELL6,	SLAVE_CMD_CONV_CELL7,	SLAVE_CMD_CONV_CELL8,	SLAVE_CMD_CONV_CELL9,	SLAVE_CMD_CONV_CELL10,	SLAVE_CMD_CONV_CELL11,	SLAVE_CMD_CONV_CELL12,	SLAVE_CMD_CONV_CLEAR,	SLAVE_CMD_CONV_SELF1,	SLAVE_CMD_CONV_SELF2,// start open wire cell conversion for all or specific	SLAVE_CMD_OWCONV_CELLS = 0x20,	SLAVE_CMD_OWCONV_CELL1,	SLAVE_CMD_OWCONV_CELL2,	SLAVE_CMD_OWCONV_CELL3,	SLAVE_CMD_OWCONV_CELL4,	SLAVE_CMD_OWCONV_CELL5,	SLAVE_CMD_OWCONV_CELL6,	SLAVE_CMD_OWCONV_CELL7,	SLAVE_CMD_OWCONV_CELL8,	SLAVE_CMD_OWCONV_CELL9,	SLAVE_CMD_OWCONV_CELL10,	SLAVE_CMD_OWCONV_CELL11,	SLAVE_CMD_OWCONV_CELL12,// start temp ADC conversion	SLAVE_CMD_CONV_TEMP = 0x30,	SLAVE_CMD_CONV_TEMP_EXT1,	SLAVE_CMD_CONV_TEMP_EXT2,	SLAVE_CMD_CONV_TEMP_INT,	SLAVE_CMD_CONV_TEMP_SELF1,	SLAVE_CMD_CONV_TEMP_SELF2,// poll adc converter status	SLAVE_CMD_POLLADC = 0x40,// poll interrupt status	SLAVE_CMD_POLLINT = 0x50,// start diagnose and poll status	SLAVE_CMD_DIAG = 0x52,
// read diagnostics register	SLAVE_CMD_READ_DIAG = 0x54,// start cell conversion with balancing permitted	SLAVE_CMD_CONV_BAL_CELLS = 0x60,	SLAVE_CMD_CONV_BAL_CELL1,	SLAVE_CMD_CONV_BAL_CELL2,	SLAVE_CMD_CONV_BAL_CELL3,	SLAVE_CMD_CONV_BAL_CELL4,	SLAVE_CMD_CONV_BAL_CELL5,	SLAVE_CMD_CONV_BAL_CELL6,	SLAVE_CMD_CONV_BAL_CELL7,	SLAVE_CMD_CONV_BAL_CELL8,	SLAVE_CMD_CONV_BAL_CELL9,	SLAVE_CMD_CONV_BAL_CELL10,	SLAVE_CMD_CONV_BAL_CELL11,	SLAVE_CMD_CONV_BAL_CELL12,// start open wire cell conversion with balancing permitted	SLAVE_CMD_OWCONV_BAL_CELLS = 0x70,	SLAVE_CMD_OWCONV_BAL_CELL1,	SLAVE_CMD_OWCONV_BAL_CELL2,	SLAVE_CMD_OWCONV_BAL_CELL3,	SLAVE_CMD_OWCONV_BAL_CELL4,	SLAVE_CMD_OWCONV_BAL_CELL5,	SLAVE_CMD_OWCONV_BAL_CELL6,	SLAVE_CMD_OWCONV_BAL_CELL7,	SLAVE_CMD_OWCONV_BAL_CELL8,	SLAVE_CMD_OWCONV_BAL_CELL9,	SLAVE_CMD_OWCONV_BAL_CELL10,	SLAVE_CMD_OWCONV_BAL_CELL11,	SLAVE_CMD_OWCONV_BAL_CELL12};#define slave_adc_active() (slave_adc_time != 0)#define slave_set_gpio(slave, gpio) slave_configs[slave][0] &= ~(0x03 << 5); slave_configs[slave][0] |= (gpio << 5)#define slave_get_gpio(slave) ((slave_configs[slave][0] >> 5) & 0x03)#define slave_set_cdc(slave, cdc) slave_configs[slave][0] &= ~0x07; slave_configs[slave][0] |= cdc#define slave_set_wdt(slave, cwdt) slave_configs[slave][0] &= ~(1 << 7); slave_configs[slave][0] |= (cwdt << 7)#define slave_set_lvlpl(slave, lvlpl) slave_configs[slave][0] &= ~(1 << 4); slave_configs[slave][0] |= (lvlpl << 4)#define slave_set_cell10(slave, cell10) slave_configs[slave][0] &= ~(1 << 3); slave_configs[slave][0] |= (cell10 << 3)#define slave_set_vuv(slave, vuv) slave_configs[slave][4] = vuv#define slave_set_vov(slave, vov) slave_configs[slave][5] = vov#define slave_set_balancing(slave, balancing) slave_configs[slave][1] = balancing & 0xFF; slave_configs[slave][2] &= 0xF0; slave_configs[slave][2] |= (balancing >> 8) & 0x0F
#define slave_set_maskint(slave, maskint) slave_configs[slave][3] = maskint & 0xFF; slave_configs[slave][2] &= 0x0F; slave_configs[slave][2] |= (maskint >> 4) & 0xF0extern volatile unsigned char slave_configs[NUMSLAVES][6];
extern volatile unsigned char slave_read_configs[NUMSLAVES][6];
extern volatile unsigned char slave_diags[NUMSLAVES][2];
extern volatile unsigned int cell_voltages[NUMCELLS];
extern volatile unsigned int cell_temps[NUMTEMPS];
extern volatile unsigned int cell_temps_raw[NUMTEMPS];
extern volatile unsigned int slave_fsm_raw[NUMSLAVES];
extern volatile unsigned int slave_fsm[NUMSLAVES];
extern volatile unsigned int ltc_temps[NUMSLAVES];
extern volatile uint32_t slave_ouv[NUMSLAVES];
extern volatile unsigned int slave_errors;
extern volatile unsigned int slave_uv_time;
extern volatile unsigned int slave_ov_time;
extern volatile unsigned int slave_ot_time;
extern volatile uint32_t batt_fsm_volt_copy;
extern volatile uint32_t batt_fsm_volt;extern volatile unsigned char slave_sent_cmd;
extern volatile unsigned int slave_cycle_ms;
extern volatile unsigned int slave_adc_time;

// voltage min/max/avg
extern volatile unsigned int slave_max_cell_volt;
extern volatile unsigned char slave_max_cell;
extern volatile unsigned int slave_min_cell_volt;
extern volatile unsigned char slave_min_cell;
extern volatile unsigned int slave_avg_cell_volt;

// temprature min/max/avg
extern volatile unsigned int slave_max_temp_val;
extern volatile unsigned char slave_max_temp;
extern volatile unsigned int slave_min_temp_val;
extern volatile unsigned char slave_min_temp;
extern volatile unsigned int slave_avg_temp_val;

unsigned char slave_loop(void);
void slave_set_balance_cell(unsigned char cell, unsigned char balance);void slave_set_gpios(char gpio);
void slave_set_cdcs(char cdc);
void slave_set_wdts(char wdt);
void slave_set_lvlpls(char lvlpl);
void slave_set_cell10s(char cell10);
void slave_set_vuvs(char vuv);
void slave_set_vovs(char vov);
void slave_set_balancings(unsigned int balancing);
void slave_set_maskints(unsigned int maskint);
void slave_init(void);
void slave_write_config(void);
void slave_start_diag(void);
void slave_read_diag(void);
void slave_start_cell_conv(char what);
void slave_start_temp_conv(void);
void slave_read_temp(void);
void slave_read_flags(void);
void slave_read_all_cellvoltages(void);
void slave_read_cellvoltages(void);
void slave_read_config(void);
void slave_verify_config(void);
unsigned char slave_tempfromadc(unsigned int adc);unsigned char slave_parse_command(void);#endif	// SLAVE_H
