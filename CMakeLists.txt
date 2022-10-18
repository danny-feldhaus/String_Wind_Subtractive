cmake_minimum_required(VERSION 3.0.0)
project(Stringwind_Subtractive VERSION 0.1.0)
set(CMAKE_CXX_COMPILER "clang++")

#include(CTest)
#enable_testing()

add_compile_definitions(DEBUG DEBUG_OVERLAP DEBUG_SCORING DEBUG_PATH_STEPS DEBUG_TIMING) #DEBUG  #DEBUG_OVERLAP

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# ### CIMG related stuff
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
include_directories( ${PROJECT_SOURCE_DIR} )
set(CIMG_H_PATH ${PROJECT_SOURCE_DIR}/src/lib/CImg)
include_directories( ${CIMG_H_PATH} )
# Flags to enable fast image display, using the XSHM library.
SET(CIMG_XSHM_CCFLAGS  -Dcimg_use_xshm)
# Flags to enable native support for PNG image files, using the PNG library.
# ( http://www.libpng.org/ )
SET(CIMG_PNG_CCFLAGS  -Dcimg_use_png)
FIND_PACKAGE(PNG)
FIND_PACKAGE(ZLIB)
FIND_PACKAGE(X11)
FIND_PACKAGE(Threads REQUIRED)
# PNG requires ZLIB

if (ZLIB_FOUND)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_ZLIB_CCFLAGS}")
  link_directories(${ZLIB_LIB_DIRS})
  include_directories(${ZLIB_INCLUDE_DIR})
  SET(SYSTEM_LIBS ${SYSTEM_LIBS} ${ZLIB_LIBRARIES})

  # PNG requires ZLIB
  if(PNG_FOUND)
    SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_PNG_CCFLAGS}")
    link_directories(${PNG_LIB_DIRS})
    include_directories(${PNG_INCLUDE_DIR} )
    SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${PNG_LIBRARIES} )
  endif()
endif()

if(X11_FOUND)
    SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_XSHM_CCFLAGS} ${CIMG_XRANDR_CCFLAGS}")
    SET(SYSTEM_LIBS ${SYSTEM_LIBS} Xext Xrandr)
endif()

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(SOURCES src/lib/image/ascii_info.hpp src/lib/image/display_manager.hpp src/lib/coordinates/coord.hpp src/lib/coordinates/line_iterator.hpp src/lib/coordinates/line.hpp src/lib/image/image_analysis.hpp src/lib/image/image_editing.hpp src/lib/coordinates/coord_cimg_plugin.h)

add_library(line_iterator src/lib/coordinates/line_iterator.cpp ${SOURCES})
add_library(string_art src/lib/image/string_art.cpp ${SOURCES})
add_executable(Stringwind_Subtractive src/main.cpp ${SOURCES})

target_include_directories(Stringwind_Subtractive PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(Stringwind_Subtractive PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/image)
target_include_directories(Stringwind_Subtractive PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/CImg)
target_include_directories(Stringwind_Subtractive PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/coordinates)
target_include_directories(Stringwind_Subtractive PRIVATE ${PNG_INCLUDE_DIR})
target_include_directories(Stringwind_Subtractive PRIVATE ${X11_INCLUDE_DIR})

target_include_directories(line_iterator PRIVATE ${CIMG_H_PATH})
target_include_directories(line_iterator PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/coordinates)

target_include_directories(string_art PRIVATE ${CIMG_H_PATH})
target_include_directories(string_art PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/coordinates)
target_include_directories(string_art PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/image)
target_include_directories(string_art PRIVATE ${PNG_INCLUDE_DIR})
target_include_directories(string_art PRIVATE ${X11_INCLUDE_DIR})

target_link_libraries(string_art line_iterator)
target_link_libraries(string_art ${PNG_LIBRARIES})
target_link_libraries(string_art ${X11_LIBRARIES})

target_link_libraries(Stringwind_Subtractive ${PNG_LIBRARIES}  ${X11_LIBRARIES}  line_iterator string_art)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
