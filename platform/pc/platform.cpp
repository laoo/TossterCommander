#include <iostream>
#include <cstdint>

uint8_t tos_image[256*1024];
uint32_t tosster_slot = 3;

uint8_t core_image[13 * 256 * 256];

extern "C" void _putchar( char c )
{
  std::cout << c;
}


extern "C"
{
void tosster_init();
uint32_t const* tosster_filter( uint8_t* rom, uint32_t addr );
}

extern "C" void tosster_put( uint8_t byte )
{
  tosster_filter( ( uint8_t* )tos_image, ( uint32_t )byte * 2 );
  tosster_filter( ( uint8_t* )tos_image, 0x200 );
}

extern "C" uint16_t tosster_get()
{
  if ( auto opt = tosster_filter( ( uint8_t* )tos_image, 0 ) )
  {
    return *opt;
  }
  else
  {
    return 0;
  }
}

extern "C" bool tosster_open()
{
  tosster_filter( (uint8_t*)tos_image, 0x100 );
  tosster_filter( (uint8_t*)tos_image, 0x200 );
  tosster_filter( (uint8_t*)tos_image, 0x000 );
  tosster_filter( (uint8_t*)tos_image, 0x200 );
  tosster_filter( (uint8_t*)tos_image, 0x200 );

  uint16_t val1 = tosster_get();
  uint16_t val2 = tosster_get();
  return  val1 != val2;
}

extern "C" void tosster_close()
{
  tosster_filter( (uint8_t*)tos_image, 0x200 );
  tosster_filter( (uint8_t*)tos_image, 0x200 );
  tosster_filter( (uint8_t*)tos_image, 0x200 );
}
