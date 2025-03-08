#include "sys.h"
#include "bl.h"
#include <string.h>
#include <stdbool.h>
#include "stm32f0xx_flash.h"


bool is_corrupted(void)
{
	uint8_t corrupt = false;
	uint8_t *u8_ptr;
	uint32_t crc32 = Crc_CalculateCRC32(
		(uint8_t *)APP_METADATA_ADDR,
		sizeof(app_metadata_s) - sizeof(app_metadata.crc),
		MAGIC_BL_FLAG
	);

	if(crc32 != app_metadata.crc) { // checking if metadata is corrupted
		corrupt = true;
	} else {
		// checking if app is corrupted
		crc32 = Crc_CalculateCRC32(
			(uint8_t *)APP_VECTOR_ADDR,
			app_metadata.size,
			MAGIC_BL_FLAG
		);
		u8_ptr = (uint8_t *) (APP_VECTOR_ADDR + app_metadata.size);
		if(memcmp(u8_ptr, &crc32, sizeof(crc32)) != 0) {
			corrupt = true;
		}
	}
	return corrupt;
}

void bl_mode(void) {
	const uint32_t flash_page_size =  0x00000400;
	const uint32_t flash_start_addr = APP_METADATA_ADDR;
	const uint32_t flash_end_addr = APP_METADATA_ADDR + APP_MAX_SIZE;
	const uint32_t num_of_pages = (flash_end_addr - flash_start_addr) / flash_page_size;
	uint32_t flash_addr = APP_METADATA_ADDR;
	uint32_t page_addr;
	uint32_t erase_counter;
	bool corrupt;
	bool blue_st = false;
	bool is_read = false;
	uint8_t rx;
	uint32_t bin = 0;
	uint8_t bin_idx = 0;
	uint8_t *bin_ptr = (uint8_t *) &bin;

	systick_init();

	FLASH_Unlock();

	/* Clear pending flags (if any) */  
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

	for(
		erase_counter = 0;
		(erase_counter < num_of_pages);
		erase_counter++
	) {
		page_addr = flash_start_addr + (flash_page_size * erase_counter);
		if (FLASH_ErasePage(page_addr)!= FLASH_COMPLETE) {
			while (1) {
				__asm__("nop");
			}
			
		}
	}

	while (1) {
		blue_st = !blue_st;
		bsp_led_set(LED_BLUE, blue_st);

		is_read = usart1_rx_to(10000, &rx);

		if(is_read) {
			FLASH_Unlock();
			bin_ptr[bin_idx] = rx;
			bin_idx++;
			if(bin_idx == sizeof(uint32_t)) {
				FLASH_ProgramWord(flash_addr, bin);
				while((*(volatile uint32_t *)flash_addr) != bin) {
					__asm__("nop");
				}
				bin_idx = 0;
				flash_addr += 4;
			}
			usart1_tx(rx);
			if(flash_addr == (APP_VECTOR_ADDR + app_metadata.size + sizeof(uint32_t))) {
				corrupt = is_corrupted();
				if(corrupt == false) {
					FLASH_Lock();
					bl_flag = 0;
					call_app_vector(app_isr_vector.Reset_Handler);
				}
			}
		} else {
			flash_addr = APP_METADATA_ADDR;
			FLASH_Lock();
		}
	}
}

int main(void) {
	usart1_io_init();
	usart1_periph_init();
	bsp_led_init();
	bsp_btn_init();
	bool corrupt = is_corrupted();

	if(corrupt || bsp_btn_is_pressed()) { // if corrupted enter bl mode
		bl_flag = MAGIC_BL_FLAG;
	}

	if(bl_flag == MAGIC_BL_FLAG) {
		bl_mode();
	} else {
		call_app_vector(app_isr_vector.Reset_Handler);
	}

	while(1) {
		__asm__("nop");
	}
}
