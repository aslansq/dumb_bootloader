#include "stm32f0xx.h"
#include <assert.h>

#define CORE_CLK_FREQ (48000000) // 48MHz
#define SYSTICK_FREQ (1000) // 1kHz
#define UINT24_MAX (0xFFFFFF)

#define GREEN GPIO_ODR_9
#define BLUE  GPIO_ODR_8
// See 00_core_clk for more details of this implementation
// SystemInit is called before main
// called from startup_stm32f051r8tx.s
void SystemInit(void) {
}

void _bsp_led_init(void) {
}

// System tick timer initialization. This is a feature of Cortex-M0 architecture. pm0215 page 4
void _systick_init(void) {
	// disable systick timer. maybe it will effect something while setting registers
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	// set current value of timer to zero
	SysTick->VAL = 0;
	// pm0215 page 4
	// System timer: SysTick is a 24-bit count-down timer
	// if requested value does not fit in 24 bits, hang processor here
	while((CORE_CLK_FREQ/SYSTICK_FREQ) >= UINT24_MAX)
		;
	// pm0215 page 65
	// RELOAD and LOAD used interchangeably between ST and ARM manuals.
	// To generate a multi-shot timer with a period of N processor clock cycles, use a RELOAD value of N-1.
	// For example, if the SysTick interrupt is required every 100 clock pulses, set RELOAD to 99.
	SysTick->LOAD = (CORE_CLK_FREQ/SYSTICK_FREQ) - 1;
	// pm0215 page 51
	// A programmable priority level of 0-192 in steps of 64 for each interrupt. A higher level corresponds to a
	// lower priority, so level 0 is the highest interrupt priority
	// so 192/64 = 3. 3 levels of priority. 0, 1, 2
	NVIC_SetPriority(SysTick_IRQn, 0); // highest priority
	// pm0215 page 63
	// The RCC feeds the Cortex System Timer (SysTick) external clock with the AHB clock
	// (HCLK) divided by 8. The SysTick can work either with that clock or directly with the Cortex
	// clock (HCLK). (STK_CSR) -> CLKSOURCE
	// choose HCLK as clock source, enable interrupt, enable systick
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

int main(void) {
	_systick_init();
	//_bsp_led_init();
	while(1)
		;
}

void NMI_Handler(void) {
	while(1)
		;
}

void HardFault_Handler(void) {
	uint32_t i = 0;
	while(1) {
		GPIOC->ODR ^= (GREEN);
		for(i = 0u; i < 1000000u; i++)
			;
	}
}

void SVC_Handler(void) {
}

void PendSV_Handler(void) {
}

uint32_t count = 0;

// 1ms timer
void SysTick_Handler(void) {
	count++;
	if(count == 200) {
		GPIOC->ODR ^= GPIO_ODR_8;
		GPIOC->ODR ^= GPIO_ODR_9;
		count = 0;
	}
}
