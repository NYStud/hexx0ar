cmake_minimum_required(VERSION 3.9)
project(hexx0ar)

set(TARGET_NAME hexx0ar)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

set(CMAKE_INCLUDE_CURRENT_DIRS ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/hexx0ar/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/hexx0ar/bin)

# packages

find_package(epoxy REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem program_options)
find_package(GLM REQUIRED)
find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2Image REQUIRED)
find_package(Lua REQUIRED)

# options

option(Debug "Debugmode" ON)

## sources / includes / linker

#TODO: remove OS specific includes // put them into if?
#openbsd
include_directories("/usr/local/include/")
include_directories("/usr/X11R6/include/")
#windows
include_directories("/mingw64/include/")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories(${LUA_INCLUDE_DIR})
include_directories(${GLM_INCLUDE_DIRS})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependecies/imgui")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependecies/sol")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependecies/json/src")
include_directories("${CMAKE_BINARY_DIR}/src/")

link_directories(${Boost_LIBRARY_DIRS})

# version number
set(VERSION_MAJOR   0   CACHE STRING "Project major version number.")
set(VERSION_MINOR   1   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   0   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

configure_file(src/application/version.hpp.in src/application/version.hpp)

## definitions

if (Debug)
    add_definitions(-DDEBUG=1)
endif ()

set(SOURCE_FILES
        dependecies/imgui/imgui_draw.cpp
        dependecies/imgui/imgui.cpp
        dependecies/imgui/imgui_demo.cpp
        src/application/application.cpp
        src/application/log.cpp
        src/application/version.hpp
        src/opengl/shaderpreprocessor.cpp
        src/opengl/shader.cpp
        src/opengl/glclasses.cpp
        src/renderer/uirenderer.cpp
        src/renderer/camera.cpp
        src/sdlwrapper/sdlwindow.cpp
        src/opengl/textures.cpp
        src/hexedit/hexedit.cpp)

set(MAIN_SRC
        src/main.cpp
        )

add_executable(${TARGET_NAME} ${SOURCE_FILES} ${MAIN_SRC})

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra)
target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:DEBUG>:-g -O2>)
target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:RELEASE>:-O3>)

target_link_libraries(${TARGET_NAME} ${EPOXY_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${LUA_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
target_link_libraries(${TARGET_NAME} pthread)
