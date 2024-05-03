#include <Windows.h>
#include <string>
#include <cstdint>
#include <vector>
#include <span>
#include <iostream>
#include <filesystem>
#include <fstream>
#define KGFLAGS_IMPLEMENTATION
#include "kgflags.h"

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
  kgflags_set_prefix( "-" );
  kgflags_set_custom_description( "TOSSTErMaker Usage:" );

  int slot = 3;
  kgflags_int( "s", 0, "TOSSTEr slot value 0-3", true, &slot );

  const char* tos_path = nullptr;
  kgflags_string( "i", "", "tos image path", true, &tos_path );

  const char* out_path = nullptr;
  kgflags_string( "o", "", "output flasher path", true, &out_path );


  if ( !kgflags_parse( argc, argv ) )
  {
    std::cout << "TOSSTErMaker\n";
    kgflags_print_errors();
    kgflags_print_usage();
    return 1;
  }

  if ( slot < 0 || slot > 3 )
  {
    std::cout << "Slot must be 0-3\n";
    return 1;
  }

  std::vector<uint8_t> templateData = getTemplate();
  auto imageSpan = findImage( templateData );
  uint8_t& slotRef = *( uint8_t* )( imageSpan.data() + imageSpan.size() + 3 );

  slotRef = slot;
  std::filesystem::path tosPath{ tos_path };

  size_t tosSize = std::filesystem::file_size( tosPath );
  if ( tosSize == 0 )
  {
    std::cout << "Tos file not found\n";
    return 1;
  }

  if ( tosSize != imageSpan.size() )
  {
    std::cout << "Tos size must be " << imageSpan.size() << " bytes\n";
    return 1;
  }

  {
    std::ifstream tosFile{ tos_path, std::ios::binary };
    tosFile.read( ( char* )imageSpan.data(), imageSpan.size() );
  }


  std::filesystem::path flasherPath{ out_path };

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
