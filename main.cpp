#include "platform.h"

int main()
{
  platform_init();
  if ( tosster_open() )
  {
    tos_puts("Tosster found\r\n");
    tosster_flash_TOS( tos_image, tosster_slot );
    tosster_close();
  }
  else
  {
    tos_puts( "No Tosster\r\n" );
  }
  return 0;
}
