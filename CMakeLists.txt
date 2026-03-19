cmake_minimum_required(VERSION 3.13)

project(codecrafters-interpreter)

set(CMAKE_CXX_STANDARD 23) 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS  src/*.cpp src/*.hpp)



add_executable(interpreter ${SOURCE_FILES})

target_include_directories(interpreter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(interpreter PRIVATE -Wall -Wextra -Werror=return-type -Wpedantic -Wshadow -Wnon-virtual-dtor)
target_link_options(interpreter PRIVATE -rdynamic)
