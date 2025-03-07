#include "bl.h"

volatile uint32_t bl_flag __attribute__((section(".bl_flag")));

app_metadata_s app_metadata __attribute__((section(".app_metadata"))) = {
	.version_major = 1,
	.version_minor = 2,
	.version_patch = 3,
	.epoch_time = 4
};

