cmake_minimum_required(VERSION 3.10)
project(myZemax)


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0")
        message(FATAL_ERROR "GCC version 7.0 or higher is required. Current version: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs")
endif()


set(STANDARD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/standard)
add_subdirectory(${STANDARD_DIR})


add_executable(myZemax
    src/main.cpp
    src/application.cpp
    src/raytracer/raytracer.cpp
    src/raytracer/camera.cpp
    src/raytracer/objects.cpp
    src/raytracer/scene.cpp
    src/ui/main_window.cpp
    src/ui/raytracer_window.cpp
    src/ui/control_panel.cpp
    src/ui/objects_panel.cpp
    src/ui/properties_window.cpp
    src/ui/add_object_dialog.cpp
    src/ui/toolbar.cpp
    src/ui/draw_overlay.cpp
)

target_include_directories(myZemax PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(myZemax PRIVATE
    hui
    dr4
    cum
)

