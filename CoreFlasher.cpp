#include "platform.h"
#include "printf.h"
#include <string_view>
#include <span>

void printCoreVersion()
{
}

int main()
{
  printf( "TOSSTEr core flasher version 1.1\r\nMatGuru & laoo/ng 2024\r\nCore version to flash:\r\n%.*s\r\n", 32, ( char const* )core_image );
  tosster_init();
  if ( tosster_open() )
  {
    auto sv = tosster_readCoreVersion();
    printf("Actual core version: %.*s\r\n", ( int )sv.size(), sv.data() );
    printf("Press \"y\" to proceed flashing\r\n" );
    char c = _getch();
    if ( c != 'y' )
    {
      printf( "Aborted\r\n" );
      tosster_close();
      return 0;
    }
    tosster_flash_core( core_image );
    tosster_close();
    printf("Press any key to exit\r\n" );
    _getch();
  }
  else
  {
    printf( "No Tosster found\r\n" );
  }
  return 0;
}
