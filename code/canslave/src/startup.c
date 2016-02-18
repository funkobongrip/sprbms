#include "main.h"

#define WEAK_ALIAS(x) __attribute__ ((weak, alias(#x)))

void Reset_Handler(void);


void Dummy_Handler(void);

void NMI_Handler(void) WEAK_ALIAS(Dummy_Handler);
void HardFault_Handler(void);
void MemManage_Handler(void) WEAK_ALIAS(Dummy_Handler);
void BusFault_Handler(void) WEAK_ALIAS(Dummy_Handler);
void UsageFault_Handler(void) WEAK_ALIAS(Dummy_Handler);
void SVC_Handler(void) WEAK_ALIAS(Dummy_Handler);
void DebugMon_Handler(void) WEAK_ALIAS(Dummy_Handler);
void PendSV_Handler(void) WEAK_ALIAS(Dummy_Handler);
void SysTick_Handler(void) WEAK_ALIAS(Dummy_Handler);

void CAN_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void SSP1_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void I2C_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void TIMER16_0_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void TIMER16_1_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void TIMER32_0_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void TIMER32_1_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void SSP0_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void UART_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void ADC_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void WDT_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void BOD_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void PIOINT3_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void PIOINT2_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void PIOINT1_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void PIOINT0_IRQHandler (void) WEAK_ALIAS(Dummy_Handler);
void WAKEUP_IRQHandler  (void) WEAK_ALIAS(Dummy_Handler);


extern void _end_stack(void);
extern uint32_t _end_text;
extern uint32_t _start_data;
extern uint32_t _end_data;
extern uint32_t _start_bss;
extern uint32_t _end_bss;
extern uint32_t _data_table;
extern uint32_t _data_table_end;

int main(void);

volatile uint32_t SystemCoreClock = 48000000;

void *vector_table[] __attribute__ ((section(".vectors"))) = {
	&_end_stack,
	Reset_Handler,
	NMI_Handler,
	HardFault_Handler,
	MemManage_Handler,
	BusFault_Handler,
	UsageFault_Handler,
	0,
	0,
	0,
	0,
	SVC_Handler,
	DebugMon_Handler,
	0,
	PendSV_Handler,
	SysTick_Handler,

	WAKEUP_IRQHandler,                      // PIO0_0  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_1  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_2  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_3  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_4  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_5  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_6  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_7  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_8  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_9  Wakeup
	WAKEUP_IRQHandler,                      // PIO0_10 Wakeup
	WAKEUP_IRQHandler,                      // PIO0_11 Wakeup
	WAKEUP_IRQHandler,                      // PIO1_0  Wakeup
	CAN_IRQHandler,							// C_CAN Interrupt
	SSP1_IRQHandler, 						// SPI/SSP1 Interrupt
	I2C_IRQHandler,                      	// I2C0
	TIMER16_0_IRQHandler,                   // CT16B0 (16-bit Timer 0)
	TIMER16_1_IRQHandler,                   // CT16B1 (16-bit Timer 1)
	TIMER32_0_IRQHandler,                   // CT32B0 (32-bit Timer 0)
	TIMER32_1_IRQHandler,                   // CT32B1 (32-bit Timer 1)
	SSP0_IRQHandler,                      	// SPI/SSP0 Interrupt
	UART_IRQHandler,                      	// UART0
	0, 				                     	// Reserved
	0,                      				// Reserved
	ADC_IRQHandler,                      	// ADC   (A/D Converter)
	WDT_IRQHandler,                      	// WDT   (Watchdog Timer)
	BOD_IRQHandler,                      	// BOD   (Brownout Detect)
	0,                      				// Reserved
	PIOINT3_IRQHandler,                     // PIO INT3
	PIOINT2_IRQHandler,                     // PIO INT2
	PIOINT1_IRQHandler,                     // PIO INT1
	PIOINT0_IRQHandler,                     // PIO INT0
};

void ClockInit()
{
	uint32_t i;
	
	// power up system oscillator
	LPC_SYSCON->PDRUNCFG &= ~(1 << 5);
	// do not bypass System Oscillator, select 10-20Mhz
	LPC_SYSCON->SYSOSCCTRL = 0;
	// wait for oscillator to settle
	for (i = 0; i < 500; i++) __NOP();

	// select System Oscillator for PLL Source
	LPC_SYSCON->SYSPLLCLKSEL  = 0x1;
	// Update Clock Source
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;
	// Toggle Update Clock Register
	LPC_SYSCON->SYSPLLCLKUEN  = 0x00;
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;
	// wait for update to complete
	while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));

	// Configure PLL, from Table 46 - UM10398, 12Mhz Crystal, 48Mhz PLL out
	LPC_SYSCON->SYSPLLCTRL = 0x23;
	// Power Up PLL
	LPC_SYSCON->PDRUNCFG &= ~(1 << 7);
	// wait until PLL is locked
	for (i = 0; i < 500; i++) __NOP();
	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));
	for (i = 0; i < 500; i++) __NOP();

	// Set Watchdog Clock and Prescaler
	//LPC_SYSCON->WDTOSCCTRL = WDTOSCCTRL_Val;
	// Power Up Watchdog
	//LPC_SYSCON->PDRUNCFG &= ~(1 << 6);

	// Select PLL as Master Clock
	LPC_SYSCON->MAINCLKSEL = 0x3;
	// Update Master Clock Source
	LPC_SYSCON->MAINCLKUEN = 0x01;
	LPC_SYSCON->MAINCLKUEN = 0x00;
	LPC_SYSCON->MAINCLKUEN = 0x01;
	// wait until change is completed
	while (!(LPC_SYSCON->MAINCLKUEN & 0x01));

	// set System Clock to PLL out clock (48Mhz)
	LPC_SYSCON->SYSAHBCLKDIV = 0x1;

	// Enable Clocks for Sys,Rom,Ram,Flashreg and GPIO
	LPC_SYSCON->SYSAHBCLKCTRL = 0x0001005F;
	
	SystemCoreClock = 48000000;

	//LPC_SYSCON->SYSMEMREMAP = SYSMEMREMAP_Val;
}

void Reset_Handler(void)
{
	uint32_t *dst, *src, *tableaddr;
	uint32_t len;
	
	tableaddr = &_data_table;
	
	while (tableaddr < &_data_table_end) {
		src = (uint32_t *)(*tableaddr++);
		dst = (uint32_t *)(*tableaddr++);
		len = (uint32_t)(*tableaddr++);
		unsigned int loop;
		for (loop = 0; loop < len; loop = loop + 4)
			*dst++ = *src++;
	}

	// Clear the bss section
	dst = &_start_bss;
	while (dst < &_end_bss)
		*dst++ = 0;
	
	ClockInit();
	
	main();
}

void Dummy_Handler(void)
{
	while(1)
	{
	}
}

void HardFault_Handler(void)
{
	while(1)
	{
	}
}
