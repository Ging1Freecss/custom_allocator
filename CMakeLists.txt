cmake_minimum_required(VERSION 3.25)

project(custom_alloc
    VERSION 0.1.0
    DESCRIPTION "Custom Allocator project in C++23"
    LANGUAGES CXX
)

# Force standard C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warning configurations target
add_library(project_warnings INTERFACE)

target_compile_options(project_warnings INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    >
)

# Source files and executable target
add_executable(custom_alloc 
    src/main.cpp
    include/custom_alloc/basic_arena.hpp
)

target_include_directories(custom_alloc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(custom_alloc
    PRIVATE
        project_warnings
)
