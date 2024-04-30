
set(CMAKE_C_COMPILE_OBJECT
  "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> -m68000 -fomit-frame-pointer -fno-common <FLAGS> -o <OBJECT> -c <SOURCE>")

set(CMAKE_C_LINK_EXECUTABLE
  "${VLINK} <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")


include(${CMAKE_ROOT}/Modules/CMakeCInformation.cmake)

set(CMAKE_C_OUTPUT_EXTENSION .o)
