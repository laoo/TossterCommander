#pragma once
#include <cstdint>
#include <string_view>
#include <span>


#ifdef _MSC_VER
#include <conio.h>
#else
extern "C" char _getch();
#endif

extern "C" {

void tosster_init();
bool tosster_open();
void tosster_close();
void tosster_flash_TOS( uint8_t const* data, uint16_t slot );
void tosster_flash_core( uint8_t const* data );

}

std::span<uint8_t> tosster_readPage( uint32_t addr );
std::string_view tosster_readCoreVersion();


extern uint8_t tos_image[256 * 1024];
extern uint32_t tosster_slot;

extern uint8_t core_image[13 * 256 * 256];
