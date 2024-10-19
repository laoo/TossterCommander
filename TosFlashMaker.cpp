#include <Windows.h>
#include <string>
#include <cstdint>
#include <vector>
#include <span>
#include <iostream>
#include <filesystem>
#include <fstream>

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
  auto pattern = "TOSIMAGE"sv;
  std::string_view view{ ( char const* )tpl.data(), tpl.size() };
  size_t begin = view.find( pattern );
  size_t end = view.rfind( pattern ) + pattern.size();
  if ( begin == std::string_view::npos || end == std::string_view::npos )
    return {};
  if ( end - begin != 256 * 1024 )
    return {};

  return std::span<uint8_t, 256 * 1024>( tpl.data() + begin, 256 * 1024 );
}


int main( int argc, char** argv )
{
  if ( argc != 3 )
  {
    std::cout << "TOSSTEr TOS Flasher Maker v1.1 by laoo/ng 2024\n";
    std::cout << "Usage:\nTossToser TOS_image_file output_flasher_file\n\nUse descriptive TOS file name as at most 32 characters from file name will be used as tos version string.\n(spaces are allowed)\n";
    return 1;
  }

  std::filesystem::path tosPath{ argv[1] };
  if ( !std::filesystem::exists( tosPath ) )
  {
    std::cout << "TOS file not found\n";
    return 1;
  }
  std::filesystem::path flasherPath{ argv[2] };

  std::vector<uint8_t> templateData = getTemplate();
  auto imageSpan = findImage( templateData );
  std::span<char,32> versionSpan = std::span<char,32>( (char*)imageSpan.data() + imageSpan.size(), 32 );

  size_t tosSize = std::filesystem::file_size( tosPath );

  if ( tosSize != imageSpan.size() )
  {
    std::cout << "Tos size must be " << imageSpan.size() << " bytes\n";
    return 1;
  }

  {
    std::ifstream tosFile{ tosPath, std::ios::binary };
    tosFile.read( ( char* )imageSpan.data(), imageSpan.size() );
  }

  tosPath.replace_extension();
  auto versionString = tosPath.filename().string();

  std::ranges::fill( versionSpan, 0 );
  std::copy_n( versionString.begin(), ( std::min )( versionString.size(), 32ull ), versionSpan.begin() );

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
