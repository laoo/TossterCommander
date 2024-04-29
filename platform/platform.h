#pragma once
#include <cstdint>

extern "C" {

void tos_puts( const char* str );
void platform_init();
bool tosster_open();
void tosster_close();
void tosster_flash_TOS( uint8_t const* data, uint16_t slot );

}

extern uint8_t tos_image[256 * 1024];
extern uint32_t tosster_slot;
