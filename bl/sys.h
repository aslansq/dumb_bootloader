#ifndef SYS_H
#define SYS_H

#include "stm32f0xx.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_BLUE ((uint32_t)GPIO_ODR_8)
#define LED_GREEN ((uint32_t)GPIO_ODR_9)

void usart1_io_init(void);
void usart1_periph_init(void);
void usart1_tx(uint8_t u8);
uint8_t usart1_rx(void);
void bsp_led_init(void);
void bsp_led_set(uint32_t led_mask, uint8_t set);
void systick_init(void);
void call_app_vector(uint32_t addr);
uint32_t Crc_CalculateCRC32(const uint8_t *s, uint32_t len, uint32_t startVal);
uint64_t sys_get_ms(void);
bool bsp_btn_is_pressed(void);
void bsp_btn_init(void);
bool usart1_rx_to(uint32_t timeout, uint8_t *rx_ptr);

#endif // SYS_H