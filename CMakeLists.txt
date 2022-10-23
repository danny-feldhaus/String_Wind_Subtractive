cmake_minimum_required(VERSION 3.0.0)
project(Stringwind_Subtractive VERSION 0.1.0)
set(CMAKE_CXX_COMPILER "clang++")

set(S_S_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17 -openmp -lpng")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DDEBUG")


file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/include/*.hpp ${PROJECT_SOURCE_DIR}/include/CImg/CImg.h)

add_subdirectory(${S_S_SOURCE_DIR}/output)
add_subdirectory(${S_S_SOURCE_DIR}/image)
add_subdirectory(${S_S_SOURCE_DIR}/coordinates)
add_executable(Stringwind_Subtractive ${S_S_SOURCE_DIR}/main.cpp ${SOURCES})
target_link_libraries(Stringwind_Subtractive string_art)
find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})
target_link_libraries(Stringwind_Subtractive ${X11_LIBRARIES})
# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
