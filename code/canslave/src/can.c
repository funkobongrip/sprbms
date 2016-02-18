#include "main.h"
#include "cr_section_macros.h"

extern void CANSendData(uint16_t addr);

ROM **rom = (ROM **)0x1fff1ff8;
uint32_t *CANCNTL = (uint32_t *)0x40050000;

// Initialize CAN Controller
uint32_t ClkInitTable[2] = {
  0x00000000UL, // CANCLKDIV
  //0x00001C42UL,  // 1000kbit
  0x00001C45UL  // 500kbit
  //0x00001C57UL  // 125kbit
};

CAN_MSG_OBJ msg_status, msg_volt14, msg_volt58, msg_volt912, msg_temp, msg_balance;

volatile uint16_t canSending = 0;
volatile uint8_t canNoSendCycles = 0;
volatile uint16_t CANBalanceCounter = 0;

// Callback function prototypes
void CAN_rx(uint8_t msg_obj_num);
void CAN_tx(uint8_t msg_obj_num);
void CAN_error(uint32_t error_info);

// Publish CAN Callback Functions
const CAN_CALLBACKS cancallbacks = {
   CAN_rx,
   CAN_tx,
   CAN_error,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
};

//	CAN receive callback
//	Function is executed by the Callback handler after
//	a CAN message has been received
void CAN_rx(uint8_t msg_obj_num)
{

  // Determine which CAN message has been received
  msg_balance.msgobj = msg_obj_num;

  // Now load up the msg_obj structure with the CAN message
  (*rom)->pCAND->can_receive(&msg_balance);

  if( msg_obj_num == 0 && (msg_balance.data[1] & 0x80) != 0 )
  {
  	  // go to bootloader
  	  ReinvokeISP();
  }
  else if( msg_obj_num == 0 )
  {
	  // Simply transmit CAN frame (echo) with with ID +0x100 via buffer 2
	  //rx_msg_obj.msgobj = 2;
	  //rx_msg_obj.mode_id += 0x100;
	  //(*rom)->pCAND->can_transmit(&rx_msg_obj);
	  BalancingStatus = msg_balance.data[0];
	  BalancingStatus |= (msg_balance.data[1] & 0xF) << 8;
	  CANBalanceCounter = 0;
  }

  return;
}

//	CAN transmit callback
//	Function is executed by the Callback handler after
//	a CAN message has been transmitted
void CAN_tx(uint8_t msg_obj_num)
{
	switch(msg_obj_num)
	{
	case 0:
		(*rom)->pCAND->can_transmit(&msg_status);
		break;
	case 1:
		(*rom)->pCAND->can_transmit(&msg_volt14);
		break;
	case 2:
		(*rom)->pCAND->can_transmit(&msg_volt58);
		break;
	case 3:
		(*rom)->pCAND->can_transmit(&msg_volt912);
		break;
	case 4:
		(*rom)->pCAND->can_transmit(&msg_temp);
		break;
	default:
		canSending = 0;
		break;
	}
	return;
}

//	CAN error callback
//	Function is executed by the Callback handler after
//	an error has occured on the CAN bus
void CAN_error(uint32_t error_info)
{
	// TODO: use error information
	if( error_info != 0 )
	{
		CANInit();
	}
	return;
}

//	CAN interrupt handler
//	The CAN interrupt handler must be provided by the user application.
//	It's function is to call the isr() API located in the ROM
void CAN_IRQHandler (void){
	(*rom)->pCAND->isr();
}

void CANStartSend(void)
{
	if( canSending == 0 )
	{
		canNoSendCycles = 0;
		canSending = 1;
		CAN_tx(0);
		ToggleGPIOBit(2, 9);
	}
	else if( canSending == 1 )
	{
		canNoSendCycles++;
		if( canNoSendCycles > MAXCANNOSENDCYCLES )
		{
			CANInit();
			setSlaveState(SLAVE_COMMERR);
		}
	}
}

void CANInit(void)
{
	// Initialize the CAN controller
	(*rom)->pCAND->init_can(&ClkInitTable[0], 1);

	// Configure the CAN callback functions */
	(*rom)->pCAND->config_calb((CAN_CALLBACKS *)&cancallbacks);
	
	// Enable the CAN Interrupt
	NVIC_EnableIRQ(CAN_IRQn);

	// configure CAN to only receive CAN_ID_BALANCE Can message
	msg_balance.msgobj		= 0;
	msg_balance.mode_id	= CAN_ID_BALANCE_OFFSET + slaveID;
	msg_balance.mask		= 0x7FF;
	(*rom)->pCAND->config_rxmsgobj(&msg_balance);
	
	msg_status.msgobj		= 1;
	msg_status.mode_id		= CAN_ID_STATUS_OFFSET + slaveID;
	msg_status.mask		= 0;
	msg_status.dlc			= 6;
	// TODO: cell status not read, set everything to 1
	msg_status.data[1]		= 0xFF;
	msg_status.data[2]		= 0x0F;

	msg_volt14.msgobj		= 2;
	msg_volt14.mode_id		= CAN_ID_VOLT14_OFFSET + (slaveID * 3);
	msg_volt14.mask		= 0;
	msg_volt14.dlc			= 6;

	msg_volt58.msgobj		= 3;
	msg_volt58.mode_id		= CAN_ID_VOLT58_OFFSET + (slaveID * 3);
	msg_volt58.mask		= 0;
	msg_volt58.dlc			= 6;

	msg_volt912.msgobj		= 4;
	msg_volt912.mode_id	= CAN_ID_VOLT912_OFFSET + (slaveID * 3);
	msg_volt912.mask		= 0;
	msg_volt912.dlc			= 6;
	
	msg_temp.msgobj			= 5;
	msg_temp.mode_id		= CAN_ID_TEMP_OFFSET + slaveID;
	msg_temp.mask			= 0;
	msg_temp.dlc			= 8;

	canSending = 0;
}

void CAN1msTimer(void)
{
	CANBalanceCounter++;
	if( CANBalanceCounter > CANBALANCEMAXMS )
	{
		CANBalanceCounter = 0;
		BalancingStatus = 0;
	}
}
