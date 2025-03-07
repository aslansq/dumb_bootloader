#include "sys.h"
#include "bl.h"
#include <string.h>
#include <stdbool.h>

int main(void) {
	usart1_io_init();
	usart1_periph_init();
	bsp_led_init();
	uint64_t time = 0;
	uint8_t st = 0;
	uint8_t is_corrupted = false;
	uint8_t *u8_ptr;

	uint32_t crc32 = Crc_CalculateCRC32(
		(uint8_t *)&app_metadata,
		sizeof(app_metadata_s) - sizeof(app_metadata.crc),
		MAGIC_BL_FLAG
	);

	if(crc32 != app_metadata.crc) { // checking if metadata is corrupted
		is_corrupted = true;
	} else {
		// checking if app is corrupted
		crc32 = Crc_CalculateCRC32(
			(uint8_t *)APP_VECTOR_ADDR,
			app_metadata.size,
			MAGIC_BL_FLAG
		);
		u8_ptr = (uint8_t *) (APP_VECTOR_ADDR + app_metadata.size);
		if(memcmp(u8_ptr, &crc32, sizeof(crc32)) != 0) {
			is_corrupted = true;
		}
	}

	if(is_corrupted) { // if corrupted enter bl mode
		bl_flag = MAGIC_BL_FLAG;
		systick_init();
	}

	if(bl_flag == MAGIC_BL_FLAG) {
		while (1) {
			if(sys_get_ms() - time > 100) {
				time = sys_get_ms();
				st = !st;
				bsp_led_set(LED_BLUE, st);
			}
		}
	} else {
		call_app_vector(app_isr_vector.Reset_Handler);
	}

	while(1) {
		__asm__("nop");
	}
}
