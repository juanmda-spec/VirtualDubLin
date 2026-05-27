# CMakeLists for the Qt5 GUI

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(virtualdub-linux
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_include_directories(virtualdub-linux PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(virtualdub-linux PUBLIC ${CMAKE_SOURCE_DIR}/src/h)

target_link_libraries(virtualdub-linux PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    vdsystem
    vdlib
)
