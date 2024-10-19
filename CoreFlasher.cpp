#include "platform.h"
#include "printf.h"
#include <string_view>
#include <span>

void printCoreVersion()
{
}

int main()
{
  printf( "TOSSTEr core flasher version 1.1\r\nMatGuru & laoo/ng 2024\r\nCore version to flash:\r\n%.*s\r\n", 31, ( char const* )core_image );
  tosster_init();
  if ( tosster_open() )
  {
    auto sv = tosster_readCoreVersion();
    //core type is encoded at index 32 of image and at index 7 of name returned from core
    // if both values are different from 0x20 they must match
    if ( core_image[32] == 0x20 || sv.second == 0x20 || core_image[32] == sv.second )
    {
      printf( "Actual core version: %.*s\r\n", ( int )sv.first.size(), sv.first.data() );
      printf( "Press \"y\" to proceed flashing\r\n" );
      char c = _getch();
      if ( c != 'y' )
      {
        printf( "Aborted\r\n" );
        tosster_close();
        return 0;
      }
      tosster_flash_core( core_image );
    }
    else
    {
      printf( "Core type does not match\r\nPlease use different flasher\r\n" );
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
