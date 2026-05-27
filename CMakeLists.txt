cmake_minimum_required(VERSION 3.10)

project(VirtualDub VERSION 1.10.4 LANGUAGES C CXX)

# Require C++14 at least for modern standards
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Platform specific definitions for Linux/Ubuntu port
if(UNIX AND NOT APPLE)
    add_compile_definitions(LINUX POSIX)
    # Temporary definintions to stub out windows.h dependencies during porting
    add_compile_definitions(_LINUX_PORT)
endif()

# We will add subdirectories here.
# For phase 1, we start by including the foundational libraries (Core)
# that need to be decoupled from Win32.

add_subdirectory(src/system)
add_subdirectory(src/VDLib)

# Main executable (stub for now, will be replaced by native UI in phase 3)
# add_subdirectory(src/VirtualDub)
add_subdirectory(src/gui-qt)
