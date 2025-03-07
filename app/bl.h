#ifndef BL_H
#define BL_H

#include <stdint.h>

// if this flag found, stay in bl mode
#define MAGIC_BL_FLAG ((uint32_t)0x1243ad78)

#define APP_METADATA_SIZE (24u)

typedef volatile struct {
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

extern volatile uint32_t bl_flag;

#endif // BL_H