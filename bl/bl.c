#include "bl.h"

volatile uint32_t bl_flag __attribute__((section(".bl_flag")));

metadata_s metadata __attribute__((section(".metadata")));

app_isr_vector_s app_isr_vector __attribute__((section(".app_vector")));
