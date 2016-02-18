#ifndef __CAN_H
#define __CAN_H

#define CAN_ID_STATUS_OFFSET		BASE_ADDR_STAT
#define CAN_ID_VOLT14_OFFSET		BASE_ADDR_VOLT
#define CAN_ID_VOLT58_OFFSET		(BASE_ADDR_VOLT+1)
#define CAN_ID_VOLT912_OFFSET		(BASE_ADDR_VOLT+2)
#define CAN_ID_TEMP_OFFSET			BASE_ADDR_TEMP
#define CAN_ID_BALANCE_OFFSET		BASE_ADDR_BALANCE

#define CONFIG_ENABLE_DRIVER_ROMCAN 1
#include "rom_drivers.h"
extern ROM **rom;

extern volatile uint16_t canSending;
extern volatile uint8_t canNoSendCycles;
extern CAN_MSG_OBJ msg_status, msg_volt14, msg_volt58, msg_volt912, msg_temp, msg_balance;

extern void CANStartSend(void);
extern void CANInit(void);
extern void CAN1msTimer(void);

#endif
