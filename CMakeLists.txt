cmake_minimum_required (VERSION 3.25)

set( TARGET_PLATFORM "ST" CACHE STRING "Platform to build for" )
option( BUILD_TOCO "Build toco" OFF )
option( BUILD_CORE_FLASHER "Build core flasher" ON )

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

include_directories( ${CMAKE_SOURCE_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/platform )

set( COMMON_FILES
  printf.c
  platform/common.cpp
)

if ( ${TARGET_PLATFORM} STREQUAL "PC" )

  set( CMAKE_CXX_STANDARD 23 )
  add_definitions( -DPLATFORM_PC )

  list( APPEND PLATFORM_FILES
    platform/pc/platform.cpp
    platform/pc/tosster.cpp
  )

  add_executable( TossToser
    TosFlashMaker.cpp
    Tostemplate.rc
  )

  add_executable( TossCorer
    CoreFlashMaker.cpp
    Coretemplate.rc
  )

elseif ( ${TARGET_PLATFORM} STREQUAL "ST" )

  set( CMAKE_CXX_STANDARD 20 )
  add_definitions( -DPLATFORM_ST -DBIG_ENDIAN )

  list( APPEND CMAKE_CXX_FLAGS "-save-temps -O3 -s" )
  list( APPEND CMAKE_C_FLAGS "-save-temps -O3 -s" )
  list( APPEND CMAKE_VASM68K_FLAGS "-quiet -Felf -spaces -nosym" )
  list( APPEND CMAKE_EXE_LINKER_FLAGS "-b ataritos -gc-all -nostdlib -s" )
  set( CMAKE_EXECUTABLE_SUFFIX ".tos" )

  set( STARTUP_FILE
    platform/st/startup.s
  )

  list( APPEND PLATFORM_FILES
    platform/st/platform.s
  )

endif()

add_executable( TosFlasher
  ${STARTUP_FILE}
  TosFlasher.cpp
  ${COMMON_FILES}
  ${PLATFORM_FILES}
  platform/st/tos_image.s
)

add_executable( CoreFlasher
  ${STARTUP_FILE}
  CoreFlasher.cpp
  ${COMMON_FILES}
  ${PLATFORM_FILES}
  platform/st/core_image.s
)

if ( ${TARGET_PLATFORM} STREQUAL "ST" )

add_custom_command( TARGET TosFlasher POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:TosFlasher> d:\\home\\hatari\\out\\HD
)

add_custom_command( TARGET CoreFlasher POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:CoreFlasher> d:\\home\\hatari\\out\\HD
)

#add_custom_command( TARGET CoreFlasher POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:CoreFlasher> H:\\
#)

endif()