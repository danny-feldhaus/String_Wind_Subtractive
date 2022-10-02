cmake_minimum_required(VERSION 3.0.0)
project(Stringwind_Subtractive VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(PNG REQUIRED)
find_package(X11 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/image)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/CImg)
include_directories(${PNG_INCLUDE_DIR})
include_directories(${X11_INCLUDE_DIR})

add_executable(Stringwind_Subtractive src/main.cpp)

target_link_libraries(Stringwind_Subtractive ${PNG_LIBRARIES})
target_link_libraries(Stringwind_Subtractive ${X11_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
