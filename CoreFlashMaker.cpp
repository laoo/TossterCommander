#include <Windows.h>
#include <algorithm>
#include <string>
#include <cstdint>
#include <vector>
#include <span>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <charconv>

#ifndef TOS_TEMPLATE
#define TOS_TEMPLATE 42
#endif

          
using namespace std::string_view_literals;

std::vector<uint8_t> getTemplate()
{
  HRSRC res = FindResource( GetModuleHandle( NULL ), MAKEINTRESOURCE( TOS_TEMPLATE ), RT_RCDATA );
  if ( !res )
    return {};

  HGLOBAL res_handle = LoadResource( NULL, res );
  if ( !res_handle )
    return {};

  uint8_t const* res_data = ( uint8_t const* )LockResource( res_handle );
  size_t res_size = SizeofResource( NULL, res );

  return std::vector<uint8_t>( res_data, res_data + res_size );
}

std::span<uint8_t> findImage( std::span<uint8_t> tpl )
{
  auto pattern = "CORIMAGE"sv;
  std::string_view view{ ( char const* )tpl.data(), tpl.size() };
  size_t begin = view.find( pattern );
  size_t end = view.rfind( pattern ) + pattern.size();
  if ( begin == std::string_view::npos || end == std::string_view::npos )
    return {};
  if ( end - begin != 13 * 256 * 256 )
    return {};

  return std::span<uint8_t, 13 * 256 * 256>( tpl.data() + begin, 13 * 256 * 256 );
}

int main( int argc, char** argv )
{
  if ( argc != 4 )
  {
    std::cout << "TOSSTEr Core Flasher Maker v1.1 by laoo/ng 2024\n";
    std::cout << "Usage:\nTossCorer core_hex_file output_flasher_file core_type_decimal_number\n\nUse descriptive core hex file name as at most 32 characters from file name will be used as core version string.\n(spaces are allowed)\n";
    return 1;
  }

  std::filesystem::path corePath{ argv[1] };
  if ( !std::filesystem::exists( corePath ) )
  {
    std::cout << "Core file not found\n";
    return 1;
  }

  std::filesystem::path flasherPath{ argv[2] };

  int coreType = atoi( argv[3] );
  if ( coreType < 1 || coreType > 255 )
  {
    std::cout << "third parameter must be a decimal number in the range 1..255\n";
    return 0;
  }

  std::vector<uint8_t> templateData = getTemplate();
  auto imageSpan = findImage( templateData );

  std::ranges::fill( imageSpan, 0xff );

  std::ifstream coreFile{ corePath };

  int offset = 0;
  uint8_t value;
  while ( !coreFile.bad() )
  {
    std::string line;
    std::getline( coreFile, line );
    if ( line.empty() )
      break;
    auto res = std::from_chars( line.data(), line.data() + line.size(), value, 16 );
    if ( res.ec == std::errc::invalid_argument )
    {
      std::cout << "Core file must be a HEX file with one value in each line\n";
      return 1;
    }
    
    imageSpan[offset++] = value;
    if ( offset >= imageSpan.size() )
    {
      std::cout << "Core file too big\n";
      return 1;
    }
  }

  if ( offset == 0 )
  {
    std::cout << "Error reading core file\n";
    return 1;
  }

  corePath.replace_extension();
  auto versionString = corePath.filename().string();

  std::fill_n( imageSpan.begin(), 256, 0 );
  std::copy_n( versionString.begin(), (std::min)( versionString.size(), 32ull ), imageSpan.begin() );
  //core type is encoded at index 32 of image
  imageSpan[32] = (uint8_t)coreType;

  std::ofstream outFile{ flasherPath, std::ios::binary };
  if ( outFile.good() )
    outFile.write( ( char* )templateData.data(), templateData.size() );
  else
  {
    std::cout << "Can't write to " << flasherPath << "\n";
    return 1;
  }

  return 0;
}
