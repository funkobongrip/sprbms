#ifndef __IAP_H
#define __IAP_H

void getDeviceUID (uint32_t UID_array[4]);
uint8_t getSlaveID(void);
void ReinvokeISP(void);

#endif
