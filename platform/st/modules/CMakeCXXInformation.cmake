
set(CMAKE_CXX_COMPILE_OBJECT
  "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> -m68000 -fno-exceptions -fomit-frame-pointer -fno-common <FLAGS> -o <OBJECT> -c <SOURCE>")

set(CMAKE_CXX_LINK_EXECUTABLE
  "${VLINK} <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

include(${CMAKE_ROOT}/Modules/CMakeCXXInformation.cmake)

set(CMAKE_CXX_OUTPUT_EXTENSION .o)
