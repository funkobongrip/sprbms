#ifndef __SSP_H__
#define __SSP_H__

/* SPI read and write buffer size */
#define SSP_BUFSIZE		16
#define FIFOSIZE		8

#define DELAY_COUNT		10
#define MAX_TIMEOUT		0xFF

	
/* SSP Status register */
#define SSPSR_TFE       (0x1<<0)
#define SSPSR_TNF       (0x1<<1) 
#define SSPSR_RNE       (0x1<<2)
#define SSPSR_RFF       (0x1<<3) 
#define SSPSR_BSY       (0x1<<4)

/* SSP CR0 register */
#define SSPCR0_DSS      (0x1<<0)
#define SSPCR0_FRF      (0x1<<4)
#define SSPCR0_SPO      (0x1<<6)
#define SSPCR0_SPH      (0x1<<7)
#define SSPCR0_SCR      (0x1<<8)

/* SSP CR1 register */
#define SSPCR1_LBM      (0x1<<0)
#define SSPCR1_SSE      (0x1<<1)
#define SSPCR1_MS       (0x1<<2)
#define SSPCR1_SOD      (0x1<<3)

/* SSP Interrupt Mask Set/Clear register */
#define SSPIMSC_RORIM   (0x1<<0)
#define SSPIMSC_RTIM    (0x1<<1)
#define SSPIMSC_RXIM    (0x1<<2)
#define SSPIMSC_TXIM    (0x1<<3)

/* SSP0 Interrupt Status register */
#define SSPRIS_RORRIS   (0x1<<0)
#define SSPRIS_RTRIS    (0x1<<1)
#define SSPRIS_RXRIS    (0x1<<2)
#define SSPRIS_TXRIS    (0x1<<3)

/* SSP0 Masked Interrupt register */
#define SSPMIS_RORMIS   (0x1<<0)
#define SSPMIS_RTMIS    (0x1<<1)
#define SSPMIS_RXMIS    (0x1<<2)
#define SSPMIS_TXMIS    (0x1<<3)

/* SSP0 Interrupt clear register */
#define SSPICR_RORIC    (0x1<<0)
#define SSPICR_RTIC     (0x1<<1)

/* RDSR status bit definition */
#define RDSR_RDY	0x01
#define RDSR_WEN	0x02

struct ssp_state_struct
{
	uint8_t cmd;
	uint8_t pec;
	uint8_t rlen;
	uint8_t wlen;
	//char name[10];
};

extern const struct ssp_state_struct SSPStates[];
extern volatile uint8_t SSPState;

#define SSPBusy()       ((LPC_SSP0->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE)
#define SSPClearFIFO()  for ( i = 0; i < 8; i++ ) {Dummy = LPC_SSP0->DR;}
#define SSPCSLow()     ClrGPIOBit(0, 2)
#define SSPCSHigh()    SetGPIOBit(0, 2)

/* If RX_INTERRUPT is enabled, the SSP RX will be handled in the ISR
SSPReceive() will not be needed. */
extern void SSP0_IRQHandler (void);
extern void SSP1_IRQHandler (void);
extern void SSP_Init(void);
extern void SSP_Send(uint8_t *Buf, uint32_t Length);
extern void SSP_Receive(uint8_t *buf, uint32_t Length);
extern void SSPBulkStart(volatile uint8_t *buf, uint32_t length);
extern void SSPBulkStop(volatile uint8_t *buf, uint32_t length);
void SSPSendCommand(void);
extern void SSPLoop(void);

#endif
