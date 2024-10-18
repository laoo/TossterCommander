#include "platform.h"
#include "printf.h"

int main()
{
  printf( "TOSSTEr TOS switcher\r\nversion 1.0 by laoo/ng 2024\r\n" );
  tosster_init();
  if ( tosster_open() )
  {
    auto sv = tosster_readCoreVersion();
    printf( "TOSSTEr core version: %.*s\r\n", ( int )sv.size(), sv.data() );
    auto actualSlot = tosster_actualSlot();
    printf( "Switch count: %d\r\n", (uint32_t)actualSlot.first );
    printf( "Actual slot: %d\r\n", (uint32_t)actualSlot.second + 1 );
    printf( "Select TOS slot to switch to\r\nor any other key to exit\r\n" );
    uint32_t descSlot = tosster_printSlots( actualSlot.second & 0xff );
    char c = _getch();
    if ( c < '1' || c > '4' )
    {
      printf( "Aborted\r\n" );
      tosster_close();
      return 0;
    }
    uint16_t slot = ( uint16_t )( c - '1' );
    printf( "Switch to slot " );
    tosster_printSlot( slot );
    printf( "Do you want to save selection? (y/-)\r\n" );
    slot |= _getch() == 'y' ? 0xc0 : 0x80;

    tosster_switch( slot );
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
