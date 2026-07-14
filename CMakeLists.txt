# /CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
project(Hermes LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

include(CTest)

if(BUILD_TESTING)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_subdirectory(src/core)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
