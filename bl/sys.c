
#include <assert.h>
#include "sys.h"

#define CORE_CLK_FREQ (48000000u) // 48MHz
#define USART1_BAUD   (9600u)
#define SYSTICK_FREQ (1000) // 1kHz
#define UINT24_MAX (0xFFFFFF)

typedef void (*app_vector_t)(void);

volatile uint64_t sys_time_ms = 0;

// See 00_core_clk for more details of this implementation
// SystemInit is called before main
// called from startup_stm32f051r8tx.s
void SystemInit(void) {
	// todo: explain why we need to do this
	// prefetch buffer enable
	FLASH->ACR |= FLASH_ACR_PRFTBE;

	uint32_t tmpreg = 0;
	// Wait HSI to be ready. we dont have any other clock source
	while((RCC->CR & RCC_CR_HSION) == 0)
		;
	while((RCC->CR & RCC_CR_HSIRDY) == 0)
		;
	// rm00091 page 111
	// zero HSITRIM
	tmpreg = RCC->CR & (~RCC_CR_HSITRIM_Msk);
	// set to default value.
	tmpreg |= (16 << RCC_CR_HSITRIM_Pos);
	// now HSI SHOULD be 8MHz
	RCC->CR = tmpreg;

	// Hmm, it should be impossible that PLL is the source of system clock
	assert(!((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL));
	// Disable PLL. We could not modify PLL config otherwise. Example:
	// rm00091 page 113
	// PLLMUL[3:0]: PLL multiplication factor
	// These bits are written by software to define the PLL multiplication factor. These bits can be
	// written only when PLL is disabled.
	RCC->CR &= (~RCC_CR_PLLON);

	// Wait for PLL to be disabled
	while((RCC->CR & RCC_CR_PLLRDY) != 0)
		;

	// reset prediv bits
	tmpreg = RCC->CFGR2 & (~RCC_CFGR2_PREDIV);
	tmpreg |= RCC_CFGR2_PREDIV_DIV2;
	// from architecture it looks like we dont need it but RCC_CFGR2_PLLSRC documentation is weird
	// lets set it to HSI/2 to be safe
	RCC->CFGR2 = tmpreg;

	// reset pll src. source is is HSI/2
	tmpreg = RCC->CFGR & (~RCC_CFGR_PLLSRC);
	// hsi div2 is the source of pll
	tmpreg |= RCC_CFGR_PLLSRC_HSI_DIV2;
	RCC->CFGR = tmpreg;
	
	// reset pll mul bits
	tmpreg = RCC->CFGR & (~RCC_CFGR_PLLMUL);
	// pll mul is 12
	tmpreg |= RCC_CFGR_PLLMUL12;
	RCC->CFGR = tmpreg;

	// sys clk source is 48MHz
	RCC->CR |= RCC_CR_PLLON;
	// wait for pll to be enabled
	while((RCC->CR & RCC_CR_PLLRDY) == 0)
		;
	// todo: explain why we need to do this
	// turns out you even need set flash latency
	FLASH->ACR |= FLASH_ACR_LATENCY;
	// wait for flash latency to be set or just wait forever I ll figure out a way to fix it
	while((FLASH->ACR & FLASH_ACR_LATENCY) == 0)
		;

	// The AHB and the APB domains maximum frequency is 48 MHz.
	// All buses are clocked at maximum frequency.
	tmpreg = RCC->CFGR & (~RCC_CFGR_PPRE);
	// APB prescaler is 1
	tmpreg |= RCC_CFGR_PPRE_DIV1;
	RCC->CFGR = tmpreg;
	tmpreg = RCC->CFGR & (~RCC_CFGR_HPRE);
	// AHB prescaler is 1
	tmpreg |= RCC_CFGR_HPRE_DIV1;
	RCC->CFGR = tmpreg;

	// finally start using PLL as system clock
	tmpreg = RCC->CFGR & (~RCC_CFGR_SW);
	tmpreg |= RCC_CFGR_SW_PLL;
	RCC->CFGR = tmpreg;
}

void usart1_io_init(void) {
	// pa9 USART1_TX
	// enabled bus
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	// see all of definitions of gpio register at RM0091 page 158
	// set mode to output
	GPIOA->MODER &= (~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10));
	// alternate function mode
	GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
	// set output type to push-pull
	GPIOA->OTYPER &= (~GPIO_OTYPER_OT_9);
	// set output type to open drain even tho it is n/a for rx
	GPIOA->OTYPER |= GPIO_OTYPER_OT_10;
	// set output speed to high
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR9 | GPIO_OSPEEDR_OSPEEDR10;
	// set pull-up/pull-down to no pull-up no pull-down
	GPIOA->PUPDR &= (~(GPIO_PUPDR_PUPDR9 | GPIO_PUPDR_PUPDR10));
	// ref stm32f051r8.pdf page 37
	//| Pin name  | AF0        | AF1       | AF2      | AF3        |
	//|-----------|------------|-----------|----------|------------|
	//| PA9       | TIM15_BKIN | USART1_TX | TIM1_CH2 | TSC_G4_IO1 |
	//| PA10      | TIM17_BKIN | USART1_RX | TIM1_CH3 | TSC_G4_IO2 |
	// RM00091 page 163
	GPIOA->AFR[1] &= (~GPIO_AFRH_AFSEL9);
	GPIOA->AFR[1] |= ((uint32_t)0b01 << GPIO_AFRH_AFSEL9_Pos);
	GPIOA->AFR[1] &= (~GPIO_AFRH_AFSEL10);
	GPIOA->AFR[1] |= ((uint32_t)0b01 << GPIO_AFRH_AFSEL10_Pos);
}

void usart1_periph_init(void) {
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	// RM0091 page 744
	// disable usart1
	USART1->CR1 &= (~USART_CR1_UE);
	// word length 8
	USART1->CR1 &= (~(USART_CR1_M | ((uint32_t)1<<28)));
	// over sampling 16, this is important when calculation baudrate value
	USART1->CR1 &= (~USART_CR1_OVER8);
	// even parity
	USART1->CR1 &= (~USART_CR1_PS);
	// enable parity
	USART1->CR1 |= USART_CR1_PCE;
	// 1 stop bit
	USART1->CR2 &= (~USART_CR2_STOP);
	// RM0091 page 716 to see how baudrate generation register value calculated.
	USART1->BRR = CORE_CLK_FREQ / USART1_BAUD;
	// enable rx and tx
	USART1->CR1 = USART_CR1_TE | USART_CR1_UE | USART_CR1_RE;
}

void usart1_tx(uint8_t u8) {
	// wait until transmit data register empty
	while((USART1->ISR & USART_ISR_TXE) == 0) {
		__asm__("nop");
	}
	USART1->TDR = u8;
}

uint8_t usart1_rx(void) {
	while((USART1->ISR & USART_ISR_RXNE) == 0) {
		__asm__("nop");
	}
	return USART1->RDR;
}

void bsp_led_init(void) {
	// ld4 : PC8 blue
	// ld3 : PC9 yellow
	// So Cortex-M0 cares about power consumption due that clock are disabled by default.
	// We need to enable GPIOC clock. RM0091 page 122
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	// see all of definitions of gpio register at RM0091 page 158
	// set mode to output
	GPIOC->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
	GPIOC->MODER |= GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;
	// set output type to push-pull
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9);
	// set output speed to high
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR8 | GPIO_OSPEEDR_OSPEEDR9;
	// set pull-up/pull-down to no pull-up no pull-down
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9);
}

void bsp_led_set(uint32_t led_mask, uint8_t set) {
	if(set) {
		GPIOC->ODR |= led_mask;
	} else {
		GPIOC->ODR &= ~led_mask;
	}
}

// System tick timer initialization. This is a feature of Cortex-M0 architecture. pm0215 page 4
void systick_init(void) {
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

void call_app_vector(uint32_t addr) {
	uint32_t *ptr = (uint32_t *)addr;
	((app_vector_t)(*ptr))();
}

uint64_t sys_get_ms(void) {
	return sys_time_ms;
}
