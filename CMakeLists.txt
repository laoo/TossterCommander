cmake_minimum_required (VERSION 3.25)

set( TARGET_PLATFORM "ST" CACHE STRING "Platform to build for" )

# Building jaguar: cmake -G "MinGW Makefiles" CMAKE_SOURCE_DIR

if ( ${TARGET_PLATFORM} STREQUAL "ST" )

set ( VASM68K c:/local/vasmm68k_mot.exe )
set ( VLINK c:/local/vlink.exe )

endif()

set( TC_LANGS C CXX )

if ( ${TARGET_PLATFORM} STREQUAL "ST" )

  set( ST_DIR ${CMAKE_SOURCE_DIR}/platform/st )
  set( CMAKE_MODULE_PATH ${ST_DIR}/modules )
  set( CMAKE_TOOLCHAIN_FILE ${CMAKE_MODULE_PATH}/m68k-elf-win.cmake )

  list( APPEND TC_LANGS VASM68K )

endif()

project ( toco ${TC_LANGS} )

include_directories( ${CMAKE_SOURCE_DIR}/src )
include_directories( ${CMAKE_SOURCE_DIR}/platform )

set( MAIN_FILES
  main.cpp
)

if ( ${TARGET_PLATFORM} STREQUAL "PC" )

  set( CMAKE_CXX_STANDARD 23 )
  add_definitions( -DPLATFORM_PC )

  list( APPEND PLATFORM_FILES
    platform/common.cpp
    platform/pc/platform.cpp
    platform/pc/tosster.cpp
  )

elseif ( ${TARGET_PLATFORM} STREQUAL "ST" )

  set( CMAKE_CXX_STANDARD 20 )
  add_definitions( -DPLATFORM_ST -DBIG_ENDIAN )

  list( APPEND CMAKE_CXX_FLAGS "-save-temps -O3 -s" )
  list( APPEND CMAKE_VASM68K_FLAGS "-quiet -Felf -spaces -nosym" )
  list( APPEND CMAKE_EXE_LINKER_FLAGS "-b ataritos -gc-all -nostdlib -s -M" )
  set( CMAKE_EXECUTABLE_SUFFIX ".prg" )

  set( STARTUP_FILE
    platform/st/startup.s
  )

  list( APPEND PLATFORM_FILES
    platform/st/platform.s
    platform/common.cpp
  )

endif()

add_executable( toco
  ${STARTUP_FILE}
  ${MAIN_FILES}
  ${PLATFORM_FILES}
)

