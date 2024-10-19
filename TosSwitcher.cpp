#include "platform.h"
#include "printf.h"

int main()
{
  printf( "TOSSTEr TOS switcher version 1.1\r\nMatGuru & laoo/ng 2024\r\n" );
  tosster_init();
  if ( tosster_open() )
  {
    auto sv = tosster_readCoreVersion();
    printf( "TOSSTEr core version: %.*s\r\n", ( int )sv.first.size(), sv.first.data() );
    auto actualSlot = tosster_actualSlot();
    printf( "Switch count: %d\r\n", (uint32_t)actualSlot.first );
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
    printf( "Press 's' to save selection.\r\nAny other key to switch TOS\r\nfor current powercycle only.\r\n" );
    slot |= 0x80;
    char s = _getch();
    if ( s == 's' || s == 'S' )
    {
      slot |= 0x40;
      printf( "Slot %c flashed\r\nPress any key to exit\r\n", c );
      _getch();
    }

    tosster_switch( slot );
    tosster_close();
  }
  else
  {
    printf( "No Tosster found\r\n" );
  }
  return 0;
}
