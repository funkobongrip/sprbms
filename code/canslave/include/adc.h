#ifndef __ADC_H
#define __ADC_H

#include "type.h"

#define ADC_OFFSET			0x10
#define ADC_INDEX			4

#define ADC_DONE			0x80000000
#define ADC_OVERRUN		0x40000000
#define ADC_ADINT			0x00010000

#define ADC_NUM			8			/* for LPC11xx */
#define ADC_CLK			240000		/* set to 240kHz */

#define ADCMIDDLE_NUM		16

#ifndef TEMPR_ADC
#define TEMPR_ADC	0
#endif

#ifndef TEMP1_ADC
#define TEMP1_ADC	0
#endif

#ifndef TEMP2_ADC
#define TEMP2_ADC	0
#endif

#ifndef TEMP3_ADC
#define TEMP3_ADC	0
#endif

#ifndef TEMP4_ADC
#define TEMP4_ADC	0
#endif

#ifndef TEMP5_ADC
#define TEMP5_ADC	0
#endif

#ifndef TEMP6_ADC
#define TEMP6_ADC	0
#endif

#ifndef TEMP7_ADC
#define TEMP7_ADC	0
#endif

extern volatile uint32_t ADCRounded[TEMPCOUNT+1];

extern uint8_t ADCReadIDValue(void);
extern void ADCInit(void);
#endif
