#include <cstdint>
#include <string_view>
#include <span>
#include "printf.h"
#include "platform.h"

uint32_t pageBuffer[128*256/4];
char slotDescs[256];

static constexpr uint8_t PAGE_COUNT = 128;

extern "C"
{

  void tosster_put( uint8_t byte );
  uint16_t tosster_get();

  static void tosster_waitAck()
  {
    uint16_t value1 = tosster_get();
    for ( uint8_t cnt = 0; cnt != 2; )
    {
      uint16_t value2 = tosster_get();
      if ( value1 == 0x5aa5 && value2 == 0xa55a || value1 == 0xa55a && value2 == 0x5aa5 )
      {
        cnt += 1;
      }
      else
      {
        value1 = value2;
      }
    }
  }

  static void tosster_setReadAddress( uint32_t address )
  {
    tosster_put( ( uint8_t )0x52 );
    tosster_put( ( uint8_t )( address >> 16 ) );
    tosster_put( ( uint8_t )( address >> 8 ) );
    tosster_put( ( uint8_t )( address ) );
    tosster_waitAck();
  }

  static void tosster_setWriteAddress( uint32_t address )
  {
    tosster_put( ( uint8_t )0x57 );
    tosster_put( ( uint8_t )( address >> 16 ) );
    tosster_put( ( uint8_t )( address >> 8 ) );
    tosster_put( ( uint8_t )( address ) );
    tosster_waitAck();
  }

  static void tosster_read()
  {
    tosster_put( ( uint8_t )0x24 );
    tosster_waitAck();
  }

  static void tosster_erase( uint8_t pageCount )
  {
    tosster_put( ( uint8_t )0x5e );
    tosster_put( pageCount );
    tosster_waitAck();
  }

  static void tosster_flash( uint8_t pageCount )
  {
    tosster_put( ( uint8_t )0x21 );
    tosster_put( pageCount );
    tosster_waitAck();
  }

  static void tosster_readData( uint8_t pageCount, uint8_t* data )
  {
    tosster_put( ( uint8_t )0x26 );
    tosster_put( pageCount );
    for ( uint16_t i = 0; i < pageCount * 256; ++i )
    {
      data[i] = tosster_get();
    }
  }

  static void tosster_writeData( uint8_t pageCount, uint8_t const* data )
  {
    tosster_put( ( uint8_t )0x66 );
    tosster_put( pageCount );
    for ( uint16_t i = 0; i < pageCount * 256; ++i )
    {
      tosster_put( data[i] );
    }
    tosster_waitAck();
  }

  static bool tosster_verifyBlock( char errChar, uint32_t const* data32 )
  {
    tosster_readData( PAGE_COUNT, ( uint8_t* )pageBuffer );
    for ( uint16_t j = 0; j < PAGE_COUNT * 256 / 4; ++j )
    {
      if ( pageBuffer[j] != data32[j] )
      {
        _putchar( errChar );
        return false;
      }
    }
    return true;
  }

  static bool tosster_doFlash( uint32_t dst, uint32_t const* data32 )
  {
    static constexpr uint16_t RETRY_COUNT = 20;

    uint16_t retry;
    for ( retry = 0; retry < RETRY_COUNT; ++retry )
    {
      tosster_writeData( PAGE_COUNT, ( uint8_t const* )data32 );
      if ( tosster_verifyBlock( '^', data32 ) )
        break;
    }
    if ( retry < RETRY_COUNT )
    {
      for ( retry = 0; retry < RETRY_COUNT; ++retry )
      {
        tosster_setWriteAddress( dst );
        tosster_erase( PAGE_COUNT );
        tosster_setWriteAddress( dst );
        tosster_flash( PAGE_COUNT );
        tosster_setReadAddress( dst );
        tosster_read();

        if ( tosster_verifyBlock( '!', data32 ) )
          break;
      }
      if ( retry < RETRY_COUNT )
      {
        printf( "OK\r\n" );
        return true;
      }
    }

    printf( "Verification failed!\r\n" );
    return false;
  }

  bool tosster_flash_TOS( uint8_t const* data, uint16_t slot )
  {
    for ( uint32_t i = 0; i < 8; ++i )
    {
      uint32_t dst = 0x100000 + slot * 0x40000 + i * 32768;
      uint32_t const* data32 = ( uint32_t const* )( data + i * 32768 );
      printf( "Flashing ... %d/8 ", i + 1 );
      if ( !tosster_doFlash( dst, data32 ) )
      {
        return false;
      }
    }
    return true;
  }

  void tosster_flash_core( uint8_t const* data )
  {
    for ( uint32_t i = 0; i < 26; ++i )
    {
      uint32_t dst = i * 32768;
      uint32_t const* data32 = ( uint32_t const* )( data + dst );
      printf( "Flashing ... %d/26 ", i + 1 );
      if ( !tosster_doFlash( dst, data32 ) )
      {
        printf( "TOSSTEr won't run now after reboot\r\nTry to address the problem\r\nand repeat flashing befor rebooting\r\n" );
        return;
      }
    }
    printf( "Done\r\n" );
  }

  static void tosster_flashPage( uint32_t dst, uint8_t const* data )
  {
    tosster_writeData( 1, data );
    tosster_setWriteAddress( dst );
    tosster_erase( 1 );
    tosster_setWriteAddress( dst );
    tosster_flash( 1 );
  }

}

std::string_view tosster_readCoreVersion()
{
  tosster_setReadAddress( 0 );
  tosster_read();
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
    memset( pageBuffer, 0, 256 );
    tosster_flashPage( dst, ( uint8_t const* )pageBuffer );

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
  tosster_flashPage( dst, (uint8_t const*)slotDescs );
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
    tosster_read();
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
