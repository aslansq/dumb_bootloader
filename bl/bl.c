#include "bl.h"

volatile uint32_t bl_flag __attribute__((section(".bl_flag")));

app_metadata_s app_metadata __attribute__((section(".app_metadata")));

app_isr_vector_s app_isr_vector __attribute__((section(".app_vector")));
