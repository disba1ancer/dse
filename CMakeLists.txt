# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

cmake_minimum_required(VERSION 3.5)
project(dse)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_WIN32_WINNT=0x0A00)

set(DSE_GNU_FLAGS_DEBUG -g3 -pedantic -Wall -Wextra -Wconversion)
set(DSE_GNU_FLAGS_RELEASE)
set(DSE_GNU_LINK_FLAGS_DEBUG)
set(DSE_GNU_LINK_FLAGS_RELEASE -mwindows)

set(DSE_MSVC_FLAGS_DEBUG)
set(DSE_MSVC_FLAGS_RELEASE)
set(DSE_MSVC_LINK_FLAGS_DEBUG)
set(DSE_MSVC_LINK_FLAGS_RELEASE -entry:mainCRTStartup -subsystem:windows)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(DSE_FLAGS_DEBUG ${DSE_GNU_FLAGS_DEBUG})
    set(DSE_FLAGS_RELEASE ${DSE_GNU_FLAGS_RELEASE})
    set(DSE_LINK_FLAGS_DEBUG ${DSE_GNU_LINK_FLAGS_DEBUG})
    set(DSE_LINK_FLAGS_RELEASE ${DSE_GNU_LINK_FLAGS_RELEASE})
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(DSE_FLAGS_DEBUG ${DSE_MSVC_FLAGS_DEBUG})
    set(DSE_FLAGS_RELEASE ${DSE_MSVC_FLAGS_RELEASE})
    set(DSE_LINK_FLAGS_DEBUG ${DSE_MSVC_LINK_FLAGS_DEBUG})
    set(DSE_LINK_FLAGS_RELEASE ${DSE_MSVC_LINK_FLAGS_RELEASE})
else()
    message(FATAL_ERROR "No support for compiller")
endif()

set (SRCS
    res/icon.ico
    res/manifest.xml
    res/res.rc
    src/math/constants.h
    src/math/mat.h
    src/math/qmath.h
    src/math/vec.h
    src/math/vmath.h
    src/notifier/make_handler.h
    src/notifier/notifier.h
    src/os/io/File.cpp
    src/os/io/File.h
    src/os/io/File_win32.cpp
    src/os/io/File_win32.h
    src/os/io/IOCP_win32.cpp
    src/os/io/IOCP_win32.h
    src/os/KeyboardKeyState.h
    src/os/loop.cpp
    src/os/loop.h
    src/os/mcursor.cpp
    src/os/mcursor.h
    src/os/PaintEventData_win32.h
    src/os/win32.h
    src/os/win32_helpers.cpp
    src/os/win32_helpers.h
    src/os/Window.cpp
    src/os/Window.h
    src/os/Window_win32.cpp
    src/os/Window_win32.h
    src/os/WindowData_win32.h
    src/os/WindowEventData_win32.h
    src/os/WindowShowCommand.h
    src/scn/Camera.cpp
    src/scn/Camera.h
    src/scn/Cube.cpp
    src/scn/Cube.h
    src/scn/IMesh.h
    src/scn/IMeshStore.h
    src/scn/Material.cpp
    src/scn/Material.h
    src/scn/Object.cpp
    src/scn/Object.h
    src/scn/Scene.cpp
    src/scn/Scene.h
    src/subsys/gl/Buffer.h
    src/subsys/gl/Context31.cpp
    src/subsys/gl/Context31.h
    src/subsys/gl/FrameBuffer.h
    src/subsys/gl/gl.cpp
    src/subsys/gl/gl.h
    src/subsys/gl/ImportedFunction.h
    src/subsys/gl/Program.h
    src/subsys/gl/RenderBuffer.h
    src/subsys/gl/Shader.h
    src/subsys/gl/Texture.h
    src/subsys/gl/TrvMvOnlyRes.h
    src/subsys/gl/VAO.h
    src/subsys/gl31_impl/binds.h
    src/subsys/gl31_impl/MeshInstance.cpp
    src/subsys/gl31_impl/MeshInstance.h
    src/subsys/gl31_impl/ObjectInstance.cpp
    src/subsys/gl31_impl/ObjectInstance.h
    src/subsys/gl31_impl/Scene.cpp
    src/subsys/gl31_impl/Scene.h
    src/subsys/gl31_impl/shaders.cpp
    src/subsys/gl31_impl/shaders.h
    src/subsys/RenderOpenGL31.cpp
    src/subsys/RenderOpenGL31.h
    src/subsys/RenderOpenGL31_impl.cpp
    src/subsys/RenderOpenGL31_impl.h
    src/threadutils/emptylock.h
    src/threadutils/ExecutionThread.cpp
    src/threadutils/ExecutionThread.h
    src/threadutils/spinlock.h
    src/threadutils/TaskState.h
    src/threadutils/ThreadPool.cpp
    src/threadutils/ThreadPool.h
    src/threadutils/ThreadPool_win32.cpp
    src/threadutils/ThreadPool_win32.h
    src/threadutils/unlock_guard.h
    src/util/Access.h
    src/util/enum_bitwise.h
    src/util/FinalStep.h
    src/util/PoolAllocator.h
    src/util/ProxyContainer.h
    src/util/ProxyIterator.h
    src/CustomPainter.cpp
    src/CustomPainter.h
    src/dse.cpp
    swal/include/swal/com.h
    swal/include/swal/enum_bitwise.h
    swal/include/swal/error.h
    swal/include/swal/gdi.h
    swal/include/swal/handle.h
    swal/include/swal/strconv.h
    swal/include/swal/window.h
    swal/include/swal/zero_or_resource.h
)


add_executable(${CMAKE_PROJECT_NAME} ${SRCS})

target_include_directories(dse PRIVATE
    src
    swal/include
    include_gl
)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:${DSE_FLAGS_DEBUG}> $<$<CONFIG:Release>:${DSE_FLAGS_RELEASE}>)
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:${DSE_LINK_FLAGS_DEBUG}> $<$<CONFIG:Release>:${DSE_LINK_FLAGS_RELEASE}>)
target_link_libraries(${CMAKE_PROJECT_NAME} opengl32 dwmapi)

