#include "platform.h"
#include "printf.h"
#include <string_view>
#include <span>

void printCoreVersion()
{
}

int main(/* int argc, char* arg */)
{
  tosster_init();
  if ( tosster_open() )
  {
    auto sv = tosster_readCoreVersion();
    printf("First 'move.b  $e00000,d0' core flasher is about to flash core:\r\n%.*s\r\n", 32, (char const*)core_image );
    printf("First 32 characters of core area are:\r\n%.*s\r\n", ( int )sv.size(), sv.data() );
    printf("If you see text, the reading is correct.\r\nPress \"y\" if you are sure to do the flashing\r\n" );
    char c = _getch();
    if ( c != 'y' )
    {
      printf( "Aborted\r\n" );
      tosster_close();
      return 0;
    }
    tosster_flash_core( core_image );
    tosster_close();
  }
  else
  {
    printf( "No Tosster\r\n" );
  }

  //printf( "%d\r\n", argc );

  //for ( uint16_t i = 0; i < argc; ++i )
  //{
  //  printf( "%c", arg[i] );
  //}                                                                                
  //printf( "\r\n" );
  //if ( tosster_open() )
  //{
  //  printCoreVersion();                                                                       
  //  //tosster_flash_TOS( tos_image, tosster_slot );                                              
  //  tosster_close();
  //}
  //else
  //{
  //  printf( "No Tosster\r\n" );
  //}
  return 0;
}
