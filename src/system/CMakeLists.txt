# CMakeLists for system core library

# Create the static library with explicit sources
add_library(vdsystem STATIC
    source/Error.cpp
    source/Fraction.cpp
    source/VDNamespace.cpp
    source/VDScheduler.cpp
    source/VDString.cpp
    source/bitmath.cpp
    source/cache.cpp
    source/cmdline.cpp
    source/cpuaccel.cpp
    source/date.cpp
    source/debug.cpp
    source/debugx86.cpp
    source/error_win32.cpp
    source/event.cpp
    source/file.cpp
    source/fileasync.cpp
    source/filestream.cpp
    source/filesys.cpp
    source/filewatcher.cpp
    source/halffloat.cpp
    source/hash.cpp
    source/int128.cpp
    source/linearalloc.cpp
    source/list.cpp
    source/log.cpp
    source/math.cpp
    source/memory.cpp
    source/process.cpp
    source/profile.cpp
    source/progress.cpp
    source/protscope.cpp
    source/refcount.cpp
    source/registry.cpp
    source/stdaccel.cpp
    source/stdafx.cpp
    source/strutil.cpp
    source/text.cpp
    source/thread.cpp
    source/thunk.cpp
    source/time.cpp
    source/tls.cpp
    source/vdalloc.cpp
    source/vdstl.cpp
    source/vdstl_hash.cpp
    source/vdstl_hashtable.cpp
    source/vectors.cpp
    source/w32assist.cpp
    source/zip.cpp
)

# Include directories
target_include_directories(vdsystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/h)
target_include_directories(vdsystem PUBLIC ${CMAKE_SOURCE_DIR}/src/h)

# Note: Many of these files currently include <windows.h> and will fail to compile on Linux.
# The actual decoupling happens in Fase 2.
