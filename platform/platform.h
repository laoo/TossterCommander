#pragma once
#include <cstdint>
#include <string_view>
#include <span>
#include <utility>


#ifdef _MSC_VER
#include <conio.h>
#else
extern "C" char _getch();
#endif

extern "C" {

void tosster_init();
bool tosster_open();
void tosster_close();
bool tosster_flash_TOS( uint8_t const* data, uint16_t slot );
void tosster_verify_TOS( uint8_t const* data, uint16_t slot );
void tosster_flash_core( uint8_t const* data );

}

std::pair<std::string_view, uint8_t> tosster_readCoreVersion();
uint32_t tosster_printSlots( uint8_t actualSlot );
void tosster_printSlot( uint16_t slotNr );
void tosster_switch( uint16_t slotNr );
void tosster_udateDescSlot( uint32_t descOff, uint16_t slot, char * tos_version );
std::pair<uint16_t, uint8_t> tosster_actualSlot( uint16_t offset = 0 );

extern uint8_t tos_image[256 * 1024];
extern char tos_version[32];

extern uint8_t core_image[13 * 256 * 256];
