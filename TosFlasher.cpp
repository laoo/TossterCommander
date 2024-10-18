#include "platform.h"
#include "printf.h"

int main()
{
  printf( "TOSSTEr TOS flasher\r\nversion 1.1 by laoo/ng 2024\r\nFlashed TOS:\r\n%.*s\r\n", 32, tos_version );
  tosster_init();
  if ( tosster_open() )
  {
    auto sv = tosster_readCoreVersion();
    printf( "TOSSTEr core version: %.*s\r\n", ( int )sv.size(), sv.data() );
    printf( "Select TOS slot to flash\r\nor any other key to exit\r\n" );
    uint32_t descSlot = tosster_printSlots();
    char c = _getch();
    if ( c < '1' || c > '4' )
    {
      printf( "Aborted\r\n" );
      tosster_close();
      return 0;
    }
    uint16_t slot = ( uint16_t )( c - '1' );
    printf( "Flashing TOS '%.*s' to slot %d\r\n", 32, tos_version, slot + 1 );
    if ( tosster_flash_TOS( tos_image, slot ) )
    {
      tosster_udateDescSlot( descSlot, slot, tos_version );
    }
    tosster_close();
    printf( "Press any key to exit\r\n" );
    _getch();
  }
  else
  {
    printf( "No Tosster found\r\n" );
  }
  return 0;
}
