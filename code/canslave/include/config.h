#ifndef __CONFIG_H
#define __CONFIG_H

#define BASE_ADDR_STAT					0x100
#define BASE_ADDR_VOLT					0x200
#define BASE_ADDR_TEMP					0x220
#define BASE_ADDR_BALANCE				0x300
#define CANBALANCEMAXMS				2000

// 3.1V is 2578, 3.5V is 2846
#define LOWER_3V3_ERROR_TRESHHOLD		2578
#define UPPER_3V3_ERROR_TRESHHOLD		2846

#define CELLCOUNT						12
#define TEMPCOUNT						4
#define MAXCANNOSENDCYCLES				100

#define TEMPR_ADC						1
#define TEMP1_ADC						7
#define TEMP2_ADC						5
#define TEMP3_ADC						3
#define TEMP4_ADC						2

#endif
