cmake_minimum_required(VERSION 3.24)
project(dse
    LANGUAGES CXX
    VERSION 0.0.1.0
    DESCRIPTION "disba1ancer's engine"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE=1 -D_UNICODE=1 -D_WIN32_WINNT=0x0A00)

include(FetchContent)
FetchContent_Declare(
    swal
    GIT_REPOSITORY https://github.com/disba1ancer/swal.git
    GIT_TAG master
    FIND_PACKAGE_ARGS
)
FetchContent_Declare(
    glbinding
    GIT_REPOSITORY https://github.com/cginternals/glbinding.git
    GIT_TAG 28d32d9bbc72aedf815f18113b0bd3aa7b354108
    FIND_PACKAGE_ARGS
)
FetchContent_Declare(
    gl_registry
    GIT_REPOSITORY https://github.com/KhronosGroup/OpenGL-Registry
    GIT_TAG main
)
FetchContent_Declare(
    egl_registry
    GIT_REPOSITORY https://github.com/KhronosGroup/EGL-Registry
    GIT_TAG main
)

set(DSE_COMPILER ${CMAKE_CXX_COMPILER_ID})

if(${DSE_COMPILER} STREQUAL "Clang")
    set(DSE_COMPILER ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
endif()

if(${DSE_COMPILER} STREQUAL "GNU")
    add_compile_options(-pedantic -Wall -Wextra -Wconversion -fcoroutines)
    message(STATUS GNU)
elseif(${DSE_COMPILER} STREQUAL "MSVC")
    add_compile_options(/W3)
endif()

set(DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS
    $<$<CXX_COMPILER_ID:GNU>:-mwindows>
    $<$<CXX_COMPILER_ID:MSVC>:/entry:mainCRTStartup;/subsystem:windows>
)

set(DSE_MULTISAMPLE "" CACHE STRING "Enable multisampling with specified level")
set(DSE_THREADPOOL_TASKS_PER_IO "16" CACHE STRING "Task count per IO and UI handling")
set(DSE_RENDER_GC_OBJECTS_PER_FRAME "16" CACHE STRING "Object count wil be checked for deletion every frame")
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    option(DSE_SUBSYSTEM_WINDOWS "Use windows subsystem while linking executable binary" ON)
endif()

configure_file(config/dse_config.h.in generated/dse_config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

add_subdirectory(headers)
add_subdirectory(core)
add_subdirectory(ogl31video)
add_subdirectory(samples)
