# CMakeLists for VDLib

add_library(vdlib STATIC
    source/Dialog.cpp
    source/Job.cpp
    source/ParameterCurve.cpp
    source/PortableRegistry.cpp
    source/UIProxies.cpp
    source/fft.cpp
    stdafx.cpp
)

target_include_directories(vdlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/h)
target_include_directories(vdlib PUBLIC ${CMAKE_SOURCE_DIR}/src/h)

target_link_libraries(vdlib PUBLIC vdsystem)
