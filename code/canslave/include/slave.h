#ifndef __SLAVE_H__
#define __SLAVE_H__

#define SLAVE_REF_MIN	1912 // (2100mV / 1.5) + 512
#define SLAVE_REF_MAX	2446 // (2900mV / 1.5) + 512

extern volatile uint8_t SlaveStatus;
extern volatile uint16_t CellVoltages[];
extern volatile uint16_t FullStackMeasurement;
extern volatile uint16_t Voltage3v3Measurement;
extern volatile uint16_t CellTemperature[(TEMPCOUNT+1)][ADCMIDDLE_NUM];
extern volatile uint16_t ChipTemperature;
extern volatile uint16_t BalancingStatus;

extern void SlaveDataRead(struct ssp_state_struct state, uint8_t *buf);
extern void SlaveDataWrite(struct ssp_state_struct state, uint8_t *buf);
extern uint8_t ConvertTempsensor(uint16_t result);

#endif
