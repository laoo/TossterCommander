#include <cstdint>
#include <string_view>
#include <span>
#include "printf.h"

uint32_t pageBuffer[128*256/4];


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

  void tosster_read()
  {
    tosster_put( ( uint8_t )0x24 );
    tosster_waitAck();
  }

  void tosster_erase( uint8_t pageCount )
  {
    tosster_put( ( uint8_t )0x5e );
    tosster_put( pageCount );
    tosster_waitAck();
  }

  void tosster_flashData( uint8_t pageCount, uint8_t const* data )
  {
    tosster_writeData( pageCount, data );
    tosster_erase( pageCount );
    tosster_flash( pageCount );
  }

  void tosster_readData( uint8_t pageCount, uint8_t* data )
  {
    tosster_read();
    tosster_put( ( uint8_t )0x26 );
    tosster_put( pageCount );
    for ( uint16_t i = 0; i < pageCount; ++i )
    {
      for ( uint16_t j = 0; j < 256; ++j )
      {
        data[i * 256 + j] = tosster_get();
      }
    }
  }

  void tosster_flash_TOS( uint8_t const* data, uint16_t slot )
  {
    printf( "Flashing TOS to slot %hd\r\n", slot );
    for ( uint16_t i = 0; i < 8; ++i )
    {
      uint32_t dst = 0x100000 + slot * 0x40000 + i * 32768;
      tosster_setWriteAddress( dst );
      tosster_flashData( 128, data + i * 32768 );
      printf( "Flashing ... %hd/8\r", i + 1 );
    }
    printf( "\r\n" );
  }

  void tosster_flash_core( uint8_t const* data )
  {
    printf( "Erasing ... 0/26" );
    for ( uint32_t i = 0; i < 26; ++i )
    {
      uint32_t dst = i * 128 * 256;
      tosster_setWriteAddress( dst );
      tosster_erase( 128 );
      printf( "\rErasing ... %d/26", i + 1 );
    }
    printf( "\r\nFlashing ... 0/26" );
    for ( uint32_t i = 0; i < 26; ++i )
    {
      uint32_t dst = i * 128 * 256;
      tosster_setWriteAddress( dst );
      tosster_flashData( 128, data + i * 32768 );
      printf( "\rFlashing ... %d/26", i + 1 );
    }
    printf( "\r\nVerifying ... 0/26" );
    uint32_t const* data32 = ( uint32_t const* )data;
    for ( uint32_t i = 0; i < 26; ++i )
    {
      uint32_t dst = i * 128 * 256;
      tosster_setReadAddress( dst );
      tosster_readData( 128, ( uint8_t* )pageBuffer );
      for ( uint32_t j = 0; j < 128 * 256 / 4; ++j )
      {
        if ( pageBuffer[j] != data32[i * 128 * 256 / 4 + j] )
        {
          printf( "\rVerification failed at %d/%d", i, j );
          return;
        }
      }
      printf( "\rVerifying ... %d/26", i + 1 );
    }
    printf( "\r\nErasing, flashing and verifying done!\r\n" );
  }
}

std::span<uint8_t> tosster_readPage( uint32_t addr )
{
  tosster_setReadAddress( addr & ~255u );
  tosster_readData( 4, ( uint8_t* )pageBuffer );
  return std::span{ (uint8_t*)&pageBuffer[addr & 255u], 32 };
}


std::string_view tosster_readCoreVersion()
{
  tosster_setReadAddress( 0 );
  tosster_readData( 1, ( uint8_t* )pageBuffer );
  return std::string_view{ ( char const* )pageBuffer, 32 };
}
