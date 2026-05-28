# CMakeLists for the Qt5 GUI

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)

# CUDA configuration
option(USE_CUDA "Enable CUDA processing" OFF) # Set to OFF locally to avoid build crash if no nvcc, but the code will handle both

set(GUI_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    JobControlDialog.cpp
    JobControlDialog.h
    FFmpegDecoder.cpp
    CudaFilter_dummy.cpp
)

if(USE_CUDA)
    # Enable CUDA language
    enable_language(CUDA)
    list(APPEND GUI_SOURCES CudaFilter.cu)
    add_compile_definitions(HAS_CUDA=1)
endif()

add_executable(virtualdub-linux ${GUI_SOURCES})

target_include_directories(virtualdub-linux PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(virtualdub-linux PUBLIC ${CMAKE_SOURCE_DIR}/src/h)
target_include_directories(virtualdub-linux PRIVATE ${FFMPEG_INCLUDE_DIRS})

target_link_libraries(virtualdub-linux PRIVATE
    ${FFMPEG_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    vdsystem
    vdlib
)

if(USE_CUDA)
    set_target_properties(virtualdub-linux PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
endif()
