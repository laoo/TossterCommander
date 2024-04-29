#include <cstdint>


extern "C"
{

void tosster_put( uint8_t byte );
uint16_t tosster_get();

void tosster_waitAck()
{
  uint16_t value1 = tosster_get();
  for ( ;; )
  {
    uint16_t value2 = tosster_get();
    if ( value1 != value2 )
    {
      break;
    }
    else
    {
      value1 = value2;
    }
  }
}

void tosster_setReadAddress( uint32_t address )
{
  tosster_put( ( uint8_t )0x52 );
  tosster_put( ( uint8_t )( address >> 16 ) );
  tosster_put( ( uint8_t )( address >> 8 ) );
  tosster_put( ( uint8_t )( address ) );
  tosster_waitAck();
}

void tosster_setWriteAddress( uint32_t address )
{
  tosster_put( ( uint8_t )0x57 );
  tosster_put( ( uint8_t )( address >> 16 ) );
  tosster_put( ( uint8_t )( address >> 8 ) );
  tosster_put( ( uint8_t )( address ) );
  tosster_waitAck();
}

void tosster_writeData( uint8_t pageCount, uint8_t const* data )
{
  tosster_put( ( uint8_t )0x66 );
  tosster_put( pageCount );
  for ( uint16_t i = 0; i < pageCount; ++i )
  {
    for ( uint16_t j = 0; j < 256; ++j )
    {
      tosster_put( data[i * 256 + j] );
    }
  }
  tosster_waitAck();
}

void tosster_flash( uint8_t pageCount )
{
  tosster_put( ( uint8_t )0x21 );
  tosster_put( pageCount );
  tosster_waitAck();
}

void tosster_erase( uint8_t pageCount )
{
  tosster_put( ( uint8_t )0x5e );
  tosster_put( pageCount );
  tosster_waitAck();
}

void tosster_flash_TOS( uint8_t const* data, uint16_t slot )
{
  for ( uint16_t i = 0; i < 8; ++i )
  {
    uint32_t dst = 0x100000 + slot * 0x40000 + i * 32768;
    tosster_setWriteAddress( dst );
    tosster_writeData( 128, data + i * 32768 );
    tosster_erase( 128 );
    tosster_flash( 128 );
  }
}

}
