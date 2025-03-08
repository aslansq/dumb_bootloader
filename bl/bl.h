#ifndef BL_H
#define BL_H

#include <stdint.h>
#include "stm32f0xx.h"

// if this flag found, stay in bl mode
#define MAGIC_BL_FLAG ((uint32_t)0x1243ad78)
#define APP_METADATA_ADDR (0x8004000u)
#define APP_MAX_SIZE (48*1024)
#define APP_VECTOR_ADDR (0x8004018u)
#define APP_METADATA_SIZE (24u)
#define APP_ISR_VECTOR (192u)

typedef volatile const struct {
	uint32_t size;
	uint8_t  version_major;
	uint8_t  version_minor;
	uint8_t  version_patch;
	uint8_t  reserved;
	uint64_t epoch_time;
	uint32_t reserved1;
	uint32_t crc;
} app_metadata_s;

_Static_assert(sizeof(app_metadata_s) == APP_METADATA_SIZE, "app_metadata_s size err");

typedef volatile const struct {
	uint32_t _estack;
	uint32_t Reset_Handler;
	uint32_t NMI_Handler;
	uint32_t HardFault_Handler;
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t SVC_Handler;
	uint32_t r7;
	uint32_t r8;
	uint32_t PendSV_Handler;
	uint32_t SysTick_Handler;
	uint32_t WWDG_IRQHandler;                   /* Window WatchDog              */
	uint32_t PVD_IRQHandler;                    /* PVD through EXTI Line detect */
	uint32_t RTC_IRQHandler;                    /* RTC through the EXTI line    */
	uint32_t FLASH_IRQHandler;                  /* FLASH                        */
	uint32_t RCC_CRS_IRQHandler;                /* RCC and CRS                  */
	uint32_t EXTI0_1_IRQHandler;                /* EXTI Line 0 and 1            */
	uint32_t EXTI2_3_IRQHandler;                /* EXTI Line 2 and 3            */
	uint32_t EXTI4_15_IRQHandler;               /* EXTI Line 4 to 15            */
	uint32_t TSC_IRQHandler;                    /* TSC                          */
	uint32_t DMA1_Channel1_IRQHandler;          /* DMA1 Channel 1               */
	uint32_t DMA1_Channel2_3_IRQHandler;        /* DMA1 Channel 2 and Channel 3 */
	uint32_t DMA1_Channel4_5_IRQHandler;        /* DMA1 Channel 4 and Channel 5 */
	uint32_t ADC1_COMP_IRQHandler;              /* ADC1, COMP1 and COMP2         */
	uint32_t TIM1_BRK_UP_TRG_COM_IRQHandler;    /* TIM1 Break, Update, Trigger and Commutation */
	uint32_t TIM1_CC_IRQHandler;                /* TIM1 Capture Compare         */
	uint32_t TIM2_IRQHandler;                   /* TIM2                         */
	uint32_t TIM3_IRQHandler;                   /* TIM3                         */
	uint32_t TIM6_DAC_IRQHandler;               /* TIM6 and DAC                 */
	uint32_t r9;                                /* Reserved                     */
	uint32_t TIM14_IRQHandler;                  /* TIM14                        */
	uint32_t TIM15_IRQHandler;                  /* TIM15                        */
	uint32_t TIM16_IRQHandler;                  /* TIM16                        */
	uint32_t TIM17_IRQHandler;                  /* TIM17                        */
	uint32_t I2C1_IRQHandler;                   /* I2C1                         */
	uint32_t I2C2_IRQHandler;                   /* I2C2                         */
	uint32_t SPI1_IRQHandler;                   /* SPI1                         */
	uint32_t SPI2_IRQHandler;                   /* SPI2                         */
	uint32_t USART1_IRQHandler;                 /* USART1                       */
	uint32_t USART2_IRQHandler;                 /* USART2                       */
	uint32_t r10;                               /* Reserved                     */
	uint32_t CEC_CAN_IRQHandler;                /* CEC and CAN                  */
	uint32_t r11;                               /* Reserved                     */
} app_isr_vector_s;

_Static_assert(sizeof(app_isr_vector_s) == APP_ISR_VECTOR, "app_isr_vector_s size err");

extern volatile uint32_t bl_flag;
extern app_metadata_s app_metadata;
extern app_isr_vector_s app_isr_vector;

#endif // BL_H
