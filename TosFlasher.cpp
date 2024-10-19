#include "platform.h"
#include "printf.h"

static void flash( uint16_t slot, uint32_t descSlot )
{
  printf( "Flashing TOS '%.*s' to slot %d\r\n", 32, tos_version, slot + 1 );
  if ( tosster_flash_TOS( tos_image, slot ) )
  {
    tosster_udateDescSlot( descSlot, slot, tos_version );
  }
}

static void verify()
{
  printf( "Select TOS slot to verify\r\nor any other key to exit\r\n" );
  char c = _getch();
  if ( c < '0' || c > '4' )
  {
    return;
  }
  else
  {
    uint16_t slot = ( uint16_t )( c - '1' );
    printf( "Verifying slot " );
    tosster_printSlot( slot );
    tosster_verify_TOS( tos_image, slot );
  }
}

int main()
{
  printf( "TOSSTEr TOS flasher version 1.1\r\nMatGuru & laoo/ng 2024\r\nTOS to flash: %.*s\r\n", 32, tos_version );
  tosster_init();
  if ( tosster_open() )
  {
    auto sv = tosster_readCoreVersion();
    printf( "Core version: %.*s\r\n", ( int )sv.size(), sv.data() );
    auto actualSlot = tosster_actualSlot();
    printf( "Select destination TOS slot:\r\n\r\n" );
    uint32_t descSlot = tosster_printSlots( actualSlot.second & 3 );
    printf( "\r\n 0 : Verify only\r\n" );
    printf( "any: other key to exit\r\n" );

    char c = _getch();
    if ( c < '0' || c > '4' )
    {
      printf( "Aborted\r\n" );
      return 0;
    }
    else
    {
      if ( c == '0' )
      {
        verify();
      }
      else
      {
        flash( ( uint16_t )( c - '1' ), descSlot );
      }
    }

    tosster_close();
  }
  else
  {
    printf( "No Tosster found\r\n" );
  }

  printf( "Press any key to exit\r\n" );
  _getch();
  return 0;
}
