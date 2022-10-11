cmake_minimum_required(VERSION 3.0.0)
project(Stringwind_Subtractive VERSION 0.1.0)

include(CTest)
enable_testing()

add_compile_definitions(DEBUG DEBUG_SCORING DEBUG_PATH_STEPS DEBUG_TIMING DEBUG_OVERLAPS)

set(CMAKE_CXX_FLAGS "-Wall -Wextra" )
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(PNG REQUIRED)
find_package(X11 REQUIRED)
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
set(SOURCES src/lib/image/output.hpp src/lib/coordinates/coord.hpp src/lib/coordinates/line_iterator.hpp src/lib/coordinates/line.hpp src/lib/image/image_editing.hpp src/lib/CImg/CImg.h src/lib/coordinates/coord_cimg_plugin.h)

add_library(line_iterator src/lib/coordinates/line_iterator.cpp ${SOURCES})
add_library(string_art src/lib/image/string_art.cpp ${SOURCES})
add_executable(Stringwind_Subtractive src/main.cpp ${SOURCES})

target_include_directories(Stringwind_Subtractive PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(Stringwind_Subtractive PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/image)
target_include_directories(Stringwind_Subtractive PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/CImg)
target_include_directories(Stringwind_Subtractive PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/coordinates)
target_include_directories(Stringwind_Subtractive PRIVATE ${PNG_INCLUDE_DIR})
target_include_directories(Stringwind_Subtractive PRIVATE ${X11_INCLUDE_DIR})

target_include_directories(line_iterator PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/CImg)
target_include_directories(line_iterator PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/coordinates)

target_include_directories(string_art PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/CImg)
target_include_directories(string_art PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/coordinates)
target_include_directories(string_art PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/image)
target_include_directories(string_art PRIVATE ${PNG_INCLUDE_DIR})
target_include_directories(string_art PRIVATE ${X11_INCLUDE_DIR})

target_link_libraries(string_art line_iterator)
target_link_libraries(string_art ${PNG_LIBRARIES})
target_link_libraries(string_art ${X11_LIBRARIES})

target_link_libraries(Stringwind_Subtractive ${PNG_LIBRARIES})
target_link_libraries(Stringwind_Subtractive ${X11_LIBRARIES})
target_link_libraries(Stringwind_Subtractive line_iterator)
target_link_libraries(Stringwind_Subtractive string_art)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
