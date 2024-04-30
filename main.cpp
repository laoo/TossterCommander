#include "platform.h"
#include "printf.h"

int main()
{
  platform_init();
  if ( tosster_open() )
  {
    printf("Tosster found\r\n");
    tosster_flash_TOS( tos_image, tosster_slot );
    tosster_close();
  }
  else
  {
    printf( "No Tosster\r\n" );
  }
  return 0;
}
