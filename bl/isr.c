#include "bl.h"
#include "sys.h"

extern volatile uint64_t sys_time_ms;

void NMI_Handler(void) {
	if(bl_flag == MAGIC_BL_FLAG) {
		while(1) {
			__asm__("nop");
		}
	} else {
		call_app_vector(app_isr_vector.NMI_Handler);
	}
}

void HardFault_Handler(void) {
	if(bl_flag == MAGIC_BL_FLAG) {
		while(1) {
			__asm__("nop");
		}
	} else {
		call_app_vector(app_isr_vector.HardFault_Handler);
	}
}

void SVC_Handler(void) {
	if(bl_flag == MAGIC_BL_FLAG) {

	} else {
		call_app_vector(app_isr_vector.SVC_Handler);
	}
}

void PendSV_Handler(void) {
	if(bl_flag == MAGIC_BL_FLAG) {

	} else {
		call_app_vector(app_isr_vector.PendSV_Handler);
	}
}

void SysTick_Handler(void) {
	if(bl_flag == MAGIC_BL_FLAG) {
		sys_time_ms++;
	} else {
		call_app_vector(app_isr_vector.SysTick_Handler);
	}
}

void WWDG_IRQHandler(void) {
	call_app_vector(app_isr_vector.WWDG_IRQHandler);
}

void RTC_IRQHandler(void) {
	call_app_vector(app_isr_vector.RTC_IRQHandler);
}

void FLASH_IRQHandler(void) {
	call_app_vector(app_isr_vector.FLASH_IRQHandler);
}

void RCC_CRS_IRQHandler(void) {
	call_app_vector(app_isr_vector.RCC_CRS_IRQHandler);
}

void EXTI0_1_IRQHandler(void) {
	call_app_vector(app_isr_vector.EXTI0_1_IRQHandler);
}

void EXTI2_3_IRQHandler(void) {
	call_app_vector(app_isr_vector.EXTI2_3_IRQHandler);
}

void EXTI4_15_IRQHandler(void) {
	call_app_vector(app_isr_vector.EXTI4_15_IRQHandler);
}

void TSC_IRQHandler(void) {
	call_app_vector(app_isr_vector.TSC_IRQHandler);
}

void DMA1_Channel1_IRQHandler(void) {
	call_app_vector(app_isr_vector.DMA1_Channel1_IRQHandler);
}

void DMA1_Channel2_3_IRQHandler(void) {
	call_app_vector(app_isr_vector.DMA1_Channel2_3_IRQHandler);
}

void DMA1_Channel4_5_IRQHandler(void) {
	call_app_vector(app_isr_vector.DMA1_Channel4_5_IRQHandler);
}

void ADC1_COMP_IRQHandler(void) {
	call_app_vector(app_isr_vector.ADC1_COMP_IRQHandler);
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
	call_app_vector(app_isr_vector.TIM1_BRK_UP_TRG_COM_IRQHandler);
}

void TIM1_CC_IRQHandler(void) {
	call_app_vector(app_isr_vector.TIM1_CC_IRQHandler);
}

void TIM2_IRQHandler(void) {
	call_app_vector(app_isr_vector.TIM2_IRQHandler);
}

void TIM3_IRQHandler(void) {
	call_app_vector(app_isr_vector.TIM3_IRQHandler);
}

void TIM6_DAC_IRQHandler(void) {
	call_app_vector(app_isr_vector.TIM6_DAC_IRQHandler);
}

void TIM14_IRQHandler(void) {
	call_app_vector(app_isr_vector.TIM14_IRQHandler);
}

void TIM15_IRQHandler(void) {
	call_app_vector(app_isr_vector.TIM15_IRQHandler);
}

void TIM16_IRQHandler(void) {
	call_app_vector(app_isr_vector.TIM16_IRQHandler);
}

void TIM17_IRQHandler(void) {
	call_app_vector(app_isr_vector.TIM17_IRQHandler);
}

void I2C1_IRQHandler(void) {
	call_app_vector(app_isr_vector.I2C1_IRQHandler);
}

void I2C2_IRQHandler(void) {
	call_app_vector(app_isr_vector.I2C2_IRQHandler);
}

void SPI1_IRQHandler(void) {
	call_app_vector(app_isr_vector.SPI1_IRQHandler);
}

void SPI2_IRQHandler(void) {
	call_app_vector(app_isr_vector.SPI2_IRQHandler);
}

void USART1_IRQHandler(void) {
	call_app_vector(app_isr_vector.USART1_IRQHandler);
}

void USART2_IRQHandler(void) {
	call_app_vector(app_isr_vector.USART2_IRQHandler);
}

void CEC_CAN_IRQHandler(void) {
	call_app_vector(app_isr_vector.CEC_CAN_IRQHandler);
}