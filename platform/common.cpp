#include <cstdint>
#include <string_view>
#include <span>
#include "printf.h"
#include "platform.h"

uint32_t pageBuffer[128*256/4];
char slotDescs[256];

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

  bool tosster_flash_TOS( uint8_t const* data, uint16_t slot )
  {
    printf( "Erasing ... 0/8" );
    for ( uint32_t i = 0; i < 8; ++i )
    {
      uint32_t dst = 0x100000 + slot * 0x40000 + i * 32768;
      tosster_setWriteAddress( dst );
      tosster_erase( 128 );
      printf( "\rErasing ... %d/8", i + 1 );
    }
    printf( "\r\nFlashing ... 0/8" );
    for ( uint32_t i = 0; i < 8; ++i )
    {
      uint32_t dst = 0x100000 + slot * 0x40000 + i * 32768;
      tosster_setWriteAddress( dst );
      tosster_flashData( 128, data + i * 32768 );
      printf( "\rFlashing ... %d/8", i + 1 );
    }
    printf( "\r\nVerifying ... 0/8" );
    uint32_t const* data32 = ( uint32_t const* )data;
    for ( uint32_t i = 0; i < 8; ++i )
    {
      uint32_t dst = 0x100000 + slot * 0x40000 + i * 32768;
      tosster_setReadAddress( dst );
      tosster_readData( 128, ( uint8_t* )pageBuffer );
      for ( uint32_t j = 0; j < 128 * 256 / 4; ++j )
      {
        if ( pageBuffer[j] != data32[i * 128 * 256 / 4 + j] )
        {
          printf( "\rVerification failed at %d/%d", i, j );
          return false;
        }
      }
      printf( "\rVerifying ... %d/8", i + 1 );
    }
    printf( "\r\nErasing, flashing and verifying done!\r\n" );
    return true;
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

std::string_view tosster_readCoreVersion()
{
  tosster_setReadAddress( 0 );
  tosster_readData( 1, ( uint8_t* )pageBuffer );
  return std::string_view{ ( char const* )pageBuffer, 32 };
}

#ifndef _MSC_VER

extern "C" void* memset( void* s, int c, size_t count )
{
  for ( uint16_t i = 0; i < count; ++i )
  {
    ( ( char* )s )[i] = c;
  }
  return 0;
}

#endif


void tosster_udateDescSlot( uint32_t descOff, uint16_t slot, char * tos_version )
{
  printf( "Updating TOS description\r\n" );

  for ( uint16_t i = 0; i < 32; ++i )
  {
    slotDescs[slot * 32 + i] = tos_version[i];
  }

  if ( descOff < 512 * 256 )
  {
    uint32_t dst = 0x0D0000 + descOff;
    tosster_setWriteAddress( dst );
    memset( pageBuffer, 0, 256 );
    tosster_flashData( 1, ( uint8_t const* )pageBuffer );

    descOff += 256;
  }

  if ( descOff >= 512 * 256 )
  {
    printf( "Whole description area is full,\r\n" );
    printf( "erasing ... 0/4" );
    for ( uint32_t i = 0; i < 4; ++i )
    {
      uint32_t dst = 0x0D0000 + i * 32768;
      tosster_setWriteAddress( dst );
      tosster_erase( 128 );
      printf( "\rerasing ... %d/4", i + 1 );
    }
    printf( ", done.\r\n" );

    descOff = 0;
  }

  uint32_t dst = 0x0D0000 + descOff;
  tosster_setWriteAddress( dst );
  tosster_flashData( 1, (uint8_t const*)slotDescs );
  printf( "Done\r\n" );
}

static bool firstNotEmptySlot()
{
  uint32_t const* slot32 = ( uint32_t const* )slotDescs;
  for ( uint16_t i = 0; i < 256/4; ++i )
  {
    if ( slot32[i] != 0 )
    {
      return true;
    }
  }

  return false;
}


static uint32_t findFirstDescSlot()
{
  for ( uint16_t i = 0; i < 512; ++i )
  {
    uint32_t off = i * 256;
    tosster_setReadAddress( 0x0D0000 + off );
    tosster_readData( 1, ( uint8_t* )slotDescs );
    if ( firstNotEmptySlot() )
      return off;
  }

  memset( slotDescs, -1, 256 );
  return 512 * 256;
}


uint32_t tosster_printSlots()
{
  uint32_t descOff = findFirstDescSlot();

  //slotDescs has actual slot description content
  for ( uint32_t i = 0; i < 4; ++i )
  {
    if ( slotDescs[i * 32] == -1 )
    {
      printf( "%d: (Empty)\r\n", i + 1 );
    }
    else
    {
      printf( "%d: %.*s\r\n", i + 1, 32, slotDescs + i * 32 );
    }
  }
  return descOff;
}
