#include "main.h"

#define WRCFG		0x01
#define STCVAD		0x10
#define STTMPAD	0x30
#define DAGN		0x52
#define RDCFG		0x02
#define	RDCVA		0x06
#define RDCVB		0x08
#define RDCVC		0x0A
#define RDTMP		0x0E
#define RDDGNR		0x54

volatile uint8_t SlaveStatus = 0;
volatile uint16_t CellVoltages[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile uint16_t FullStackMeasurement = 0;
volatile uint16_t Voltage3v3Measurement = 0;
volatile uint16_t ChipTemperature = 0;
volatile uint16_t BalancingStatus = 0;

void SlaveParseCellVoltages(uint8_t offset, uint8_t *buf)
{
	uint16_t tmp1, tmp2;
	tmp1 = buf[0];
	tmp2 = buf[1];
	CellVoltages[offset] = ((tmp2 << 8) & 0xF00) | tmp1;

	tmp1 = buf[1];
	tmp2 = buf[2];
	CellVoltages[offset+1] = ((tmp2 << 4) & 0xFF0) | ((tmp1 >> 4) & 0xF);

	tmp1 = buf[3];
	tmp2 = buf[4];
	CellVoltages[offset+2] = ((tmp2 << 8) & 0xF00) | tmp1;

	tmp1 = buf[4];
	tmp2 = buf[5];
	CellVoltages[offset+3] = ((tmp2 << 4) & 0xFF0) | ((tmp1 >> 4) & 0xF);
}

void SlaveDataRead(struct ssp_state_struct state, uint8_t *buf)
{
	uint32_t tmp;
	switch(state.cmd)
	{
		case RDCFG:
		{
			if( buf[0] != 0x11 )
			{
				setSlaveState(SLAVE_COMMERR);
			}
			break;
		}
		case RDCVA:
		{
			SlaveParseCellVoltages(0, buf);
			msg_volt14.data[0] = (CellVoltages[0] & 0xFF);
			msg_volt14.data[1] = ((CellVoltages[1] & 0xF) << 4) | ((CellVoltages[0] & 0xF00) >> 8);
			msg_volt14.data[2] = (CellVoltages[1] & 0xFF0) >> 4;
			msg_volt14.data[3] = (CellVoltages[2] & 0xFF);
			msg_volt14.data[4] = ((CellVoltages[3] & 0xF) << 4) | ((CellVoltages[2] & 0xF00) >> 8);
			msg_volt14.data[5] = (CellVoltages[3] & 0xFF0) >> 4;
			break;
		}
		case RDCVB:
		{
			SlaveParseCellVoltages(4, buf);
			msg_volt58.data[0] = (CellVoltages[4] & 0xFF);
			msg_volt58.data[1] = ((CellVoltages[5] & 0xF) << 4) | ((CellVoltages[4] & 0xF00) >> 8);
			msg_volt58.data[2] = (CellVoltages[5] & 0xFF0) >> 4;
			msg_volt58.data[3] = (CellVoltages[6] & 0xFF);
			msg_volt58.data[4] = ((CellVoltages[7] & 0xF) << 4) | ((CellVoltages[6] & 0xF00) >> 8);
			msg_volt58.data[5] = (CellVoltages[7] & 0xFF0) >> 4;
			break;
		}
		case RDCVC:
		{
			SlaveParseCellVoltages(8, buf);
			msg_volt912.data[0] = (CellVoltages[8] & 0xFF);
			msg_volt912.data[1] = ((CellVoltages[9] & 0xF) << 4) | ((CellVoltages[8] & 0xF00) >> 8);
			msg_volt912.data[2] = (CellVoltages[9] & 0xFF0) >> 4;
			msg_volt912.data[3] = (CellVoltages[10] & 0xFF);
			msg_volt912.data[4] = ((CellVoltages[11] & 0xF) << 4) | ((CellVoltages[10] & 0xF00) >> 8);
			msg_volt912.data[5] = (CellVoltages[11] & 0xFF0) >> 4;
			break;
		}
		case RDTMP:
		{
			int32_t intt;
			FullStackMeasurement = buf[1]; // VTEMP1
			FullStackMeasurement = ((FullStackMeasurement << 8) & 0xF00) | buf[0];
			// 3.3V should be in range of 3.1V to 3.5V
			Voltage3v3Measurement = buf[2]; // VTEMP2
			Voltage3v3Measurement = (Voltage3v3Measurement << 4) | ((buf[1] >> 4) & 0xF);
			if( Voltage3v3Measurement < LOWER_3V3_ERROR_TRESHHOLD ||
				Voltage3v3Measurement > UPPER_3V3_ERROR_TRESHHOLD )
			{
				setSlaveState(SLAVE_POWERR);
			}
			// data[0] = status, data[1] = states, half data[2] = states
			// rest half data[2] = 4 LSB of FSM
			msg_status.data[2] &= 0x0F;
			msg_status.data[2] |= (FullStackMeasurement & 0xF) << 4;
			// data[3] 8 MSB of FSM
			msg_status.data[3] = (FullStackMeasurement & 0xFF0) >> 4;
			intt = buf[4] & 0xF;
			intt <<= 8;
			intt |= buf[3];
			// the factors are choosen so that the division can be a shifted
			intt -= 512;
			intt *= 3;
			intt /= 2;
			intt >>= 4;
			ChipTemperature = intt;
			if( ChipTemperature < 253 )
				msg_temp.data[4] = 0;
			else if( ChipTemperature > 506 )
				msg_temp.data[4] = 0xFF;
			else
			{
				tmp = ChipTemperature - 253;
				msg_temp.data[4] = tmp & 0xFF;
			}
			
			if( buf[4] & (1<<4) )
			{
				// overtemperature
				setSlaveState(SLAVE_OVERTEMP);
			}
			break;
		}
		case RDDGNR:
		{
			uint32_t ref;
			ref = buf[1] & 0xF;
			ref = ref << 8;
			ref |= buf[0];
			if( ref < SLAVE_REF_MIN || ref > SLAVE_REF_MAX )
			{
				setSlaveState(SLAVE_IC_FAIL);
			}

			if( buf[1] & (1<<5) )
				setSlaveState(SLAVE_IC_FAIL);

			// TODO: check muxfail
			CANStartSend();
			break;
		}
	}
}

void SlaveDataWrite(struct ssp_state_struct state, uint8_t *buf)
{
	buf[0] = 0x11; // CDC = 1, LVLPL = 1
	if( SlaveStatus & SLAVE_ERR )
	{
		buf[1] = 0;
		buf[2] = 0;
	}
	else
	{
		buf[1] = BalancingStatus & 0xFF;
		buf[2] = (BalancingStatus & 0xF00) >> 8;
	}
	buf[3] = 0;
	buf[4] = 0;
	buf[5] = 0;
	if( buf[1] != 0 || (buf[2] & 0xF) != 0 )
		setSlaveState(SLAVE_BALANCING);
	else
		setSlaveState(0);
}
