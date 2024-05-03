
#include <cstdint>
#include <vector>
#include <optional>
#include <array>
#include <span>
#include <Windows.h>

namespace tosster
{

enum State
{
  IDLE = 0,
  READ_ADDR0,
  READ_ADDR1,
  READ_ADDR2,
  WRITE_ADDR0,
  WRITE_ADDR1,
  WRITE_ADDR2,
  BUFFER_WRITE0,
  BUFFER_WRITE1,
  BUFFER_READ0,
  BUFFER_READ1,
  FLASH_WRITE0,
  FLASH_ERASE0,
  SELECT_SLOT0,
  WAIT_FOR_ACK
};

static constexpr uint8_t CMD_READ_ADDR = 0x52;
static constexpr uint8_t CMD_WRITE_ADDR = 0x57;
static constexpr uint8_t CMD_BUFFER_WRITE = 0x66;
static constexpr uint8_t CMD_BUFFER_READ = 0x26;  //DOC IS WRONG
static constexpr uint8_t CMD_FLASH_WRITE = 0x21;
static constexpr uint8_t CMD_FLASH_READ = 0x24;
static constexpr uint8_t CMD_FLASH_ERASE = 0x5e;
static constexpr uint8_t CMD_SELECT_SLOT = 0x13;

static std::span<uint8_t> gData;
static std::vector<uint8_t> gBuffer;

static uint32_t gReadAddr = 0;
static uint32_t gWriteAddr = 0;
static uint32_t gBufferLimit = 0;
static uint32_t gBufferIndex = 0;
static uint32_t gCurrentSlot = 0;

static State gState = IDLE;

static uint8_t* mAllocBase = nullptr;
static HANDLE hFile = NULL;
static HANDLE hMapFile = NULL;


static void init()
{
  gBuffer.resize( 32768, 0xff );

  hFile = CreateFile( "../../../TOSSTer.dat",
    GENERIC_READ | GENERIC_WRITE,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL );

  if ( hFile == INVALID_HANDLE_VALUE )
    return;

  hMapFile = CreateFileMapping( hFile,          // current file handle
    NULL,           // default security
    PAGE_READWRITE, // read/write permission
    0,              // size of mapping object, high
    2 * 1024 * 1024,     // size of mapping object, low
    NULL );         // name of mapping object

  if ( hMapFile == NULL )
    return;

  mAllocBase = ( uint8_t* )MapViewOfFileEx( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 2 * 1024 * 1024, ( void* )0xE00000 );

  if ( mAllocBase == NULL )
  {
    return;
  }

  gData = std::span<uint8_t>( mAllocBase, 2 * 1024 * 1024 );
}

static void readFlash()
{
  uint8_t const* src = gData.data() + ( gReadAddr & 0x1fffff );
  memcpy( gBuffer.data(), src, gBuffer.size() );
}

static void flash()
{
  uint8_t * dst = gData.data() + ( gWriteAddr & 0x1fffff );
  memcpy( dst, gBuffer.data(), gBufferLimit );
}

static void erase()
{
  if ( gBufferLimit == 0x8000 )
    gWriteAddr &= ~0b1111;

  uint8_t * dst = gData.data() + ( gWriteAddr & 0x1fffff );
  memset( dst, 0xff, gBufferLimit );
}

static void updateRom( uint8_t* rom )
{
  uint8_t const* src = gData.data() + 0x100000 + gCurrentSlot * 0x40000;
  memcpy( rom, src, 0x40000 );
}

static uint16_t read( uint8_t addr )
{
  static constexpr std::array<uint16_t, 2> idle = { 0xa55a, 0x5aa5 };
  static uint32_t accesIdx = 0;

  switch ( gState )
  {
  case WAIT_FOR_ACK:
    gState = IDLE;
    return 0x2e2e;
  case BUFFER_WRITE1:
    return gBufferLimit - gBufferIndex;
  case BUFFER_READ1:
    if ( gBufferIndex + 1 >= gBufferLimit )
      gState = IDLE;
    return gBuffer[gBufferIndex++];
  default:
    break;
  }

  return idle[accesIdx++ & 1];
}

static void write( uint8_t data )
{
  switch ( gState )
  {
  case READ_ADDR0:
    gReadAddr = data << 16;
    gState = READ_ADDR1;
    break;
  case READ_ADDR1:
    gReadAddr |= data << 8;
    gState = READ_ADDR2;
    break;
  case READ_ADDR2:
    gReadAddr |= data;
    gState = WAIT_FOR_ACK;
    break;
  case WRITE_ADDR0:
    gWriteAddr = data << 16;
    gState = WRITE_ADDR1;
    break;
  case WRITE_ADDR1:
    gWriteAddr |= data << 8;
    gState = WRITE_ADDR2;
    break;
  case WRITE_ADDR2:
    gWriteAddr |= data;
    gState = WAIT_FOR_ACK;
    break;
  case BUFFER_WRITE0:
    gBufferLimit = data << 8;
    gBufferIndex = 0;
    gState = gBufferLimit == 0 ? WAIT_FOR_ACK : BUFFER_WRITE1;
    break;
  case BUFFER_WRITE1:
    gBuffer[gBufferIndex++] = data;
    if ( gBufferIndex >= gBufferLimit )
      gState = WAIT_FOR_ACK;
    break;
  case BUFFER_READ0:
    gBufferLimit = data << 8;
    gBufferIndex = 0;
    gState = gBufferLimit == 0 ? IDLE : BUFFER_READ1;
    break;
  case BUFFER_READ1:
    break;
  case FLASH_WRITE0:
    gBufferLimit = data << 8;
    flash();
    gState = WAIT_FOR_ACK;
    break;
  case FLASH_ERASE0:
    gBufferLimit = data << 8;
    erase();
    gState = WAIT_FOR_ACK;
    break;
  case SELECT_SLOT0:
    gCurrentSlot = data & 0b11;
    gState = WAIT_FOR_ACK;
    break;
  default:  //IDLE
    switch ( data )
    {
    case CMD_READ_ADDR:
      gState = READ_ADDR0;
      break;
    case CMD_WRITE_ADDR:
      gState = WRITE_ADDR0;
      break;
    case CMD_BUFFER_WRITE:
      gState = BUFFER_WRITE0;
      break;
    case CMD_BUFFER_READ:
      gState = BUFFER_READ0;
      break;
    case CMD_FLASH_WRITE:
      gState = FLASH_WRITE0;
      break;
    case CMD_FLASH_READ:
      readFlash();
      gState = WAIT_FOR_ACK;
      break;
    case CMD_FLASH_ERASE:
      gState = FLASH_ERASE0;
      break;
    case CMD_SELECT_SLOT:
      gState = SELECT_SLOT0;
      break;
    default:
      break;
    }
    break;
  }
}


static std::optional<uint16_t> filter( uint8_t* rom, uint32_t addr )
{
  static uint32_t magicIndex = 0;
  static std::optional<uint8_t> value = std::nullopt;

  if ( addr > 0x100 )
    return std::nullopt;

  switch ( magicIndex )
  {
  case 0:
    if ( addr == 0x080 )
      magicIndex = 1;
    break;
  case 1:
    if ( addr == 0x100 )
      magicIndex = 2;
    else
      magicIndex = 0;
    break;
  case 2:
    if ( addr == 0x000 )
      magicIndex = 3;
    else
      magicIndex = 0;
    break;
  case 3:
    if ( addr == 0x100 )
      magicIndex = 4;
    else
      magicIndex = 0;
    break;
  case 4:
    if ( addr == 0x100 )
      magicIndex = 5;
    else
      magicIndex = 0;
    break;
  default:
    if ( addr == 0x100 )
    {
      magicIndex = ( magicIndex + 1 ) & 7;
      if ( value )
        write( ( uint8_t )*value );
      if ( magicIndex == 0 )
        updateRom( rom );
      break;
    }
    else
    {
      magicIndex = 5;
      value = addr & 0xff;
      return read( ( uint8_t )addr );
    }
  }

  return std::nullopt;
}

}

extern uint8_t core_image[13 * 256 * 256];

extern "C"
{


void tosster_init()
{
  static const char dat[9] = "CORIMAGE";

  for ( size_t i = 0; i < 32768; ++i )
  {
    memcpy( core_image + i * 8, dat, 8 );
  }
  tosster::init();
}

uint32_t const* tosster_filter( uint8_t* rom, uint32_t addr )
{
  static uint32_t value;

  if ( auto opt = tosster::filter( rom, addr / 2 ) )
  {
    value = *opt;
    return &value;
  }

  return nullptr;
}

}


