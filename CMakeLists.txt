cmake_minimum_required(VERSION 3.5)
project(imgui)

set(SOURCES
../../backends/imgui_impl_dx11.cpp 
../../backends/imgui_impl_win32.cpp 
)

file(GLOB MORE_SOURCES "../../imgui*.cpp")
list(APPEND SOURCES ${MORE_SOURCES})

include_directories(
    ../..
    ../../backends
)

add_library(imgui STATIC ${SOURCES})