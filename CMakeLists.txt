cmake_minimum_required(VERSION 3.5)
project(dse
    LANGUAGES CXX
    VERSION 1.0
    DESCRIPTION "DSE Engine"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_WIN32_WINNT=0x0A00)

set(DSE_CXX_GNU $<COMPILE_LANG_AND_ID:CXX,GNU>)
set(DSE_CXX_MSVC $<COMPILE_LANG_AND_ID:CXX,MSVC>)
set(DSE_CXX_FLAGS
    $<$<BOOL:${DSE_CXX_GNU}>:-pedantic;-Wall;-Wextra;-Wconversion;-fcoroutines>
#    $<$<BOOL:${DSE_CXX_MSVC}>:/W3>
)
set(DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS
    $<$<CXX_COMPILER_ID:GNU>:-mwindows>
    $<$<CXX_COMPILER_ID:MSVC>:/entry:mainCRTStartup;/subsystem:windows>
)

set(DSE_WIN32_SRCS
    rc_win32/icon.ico
    rc_win32/manifest.xml
    rc_win32/res.rc
    src/core/File_win32.cpp
    src/core/File_win32.h
#    src/os/io/IOCP_win32.cpp
#    src/os/io/IOCP_win32.h
    src/os/PaintEventData_win32.h
    src/os/win32.h
    src/os/Window_win32.cpp
    src/os/Window_win32.h
    src/os/WindowData_win32.h
    src/os/WindowEventData_win32.h
    src/core/ThreadPool_win32.cpp
    src/core/ThreadPool_win32.h
    src/core/errors_win32.cpp
    src/core/errors_win32.h
    swal/include/swal/com.h
    swal/include/swal/enum_bitwise.h
    swal/include/swal/error.h
    swal/include/swal/gdi.h
    swal/include/swal/handle.h
    swal/include/swal/strconv.h
    swal/include/swal/window.h
    swal/include/swal/zero_or_resource.h
    swal/include/swal/win_headers.h
)

set (SRCS
    src/math/constants.h
    src/math/mat.h
    src/math/qmath.h
    src/math/vec.h
    src/math/vmath.h
    src/notifier/notifier.h
    src/core/File.cpp
    src/core/File.h
    src/os/KeyboardKeyState.h
    src/os/loop.cpp
    src/os/loop.h
    src/os/mcursor.cpp
    src/os/mcursor.h
    src/os/Window.cpp
    src/os/Window.h
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
    src/util/emptylock.h
    src/util/ExecutionThread.cpp
    src/util/ExecutionThread.h
    src/util/spinlock.h
    src/util/TaskState.h
    src/core/ThreadPool.cpp
    src/core/ThreadPool.h
    src/core/errors.cpp
    src/core/errors.h
    src/util/unlock_guard.h
    src/util/access.h
    src/util/enum_bitwise.h
    src/util/scope_exit.h
    src/util/pimpl.h
    src/util/PoolAllocator.h
    src/util/ProxyContainer.h
    src/util/ProxyIterator.h
    src/util/functional.h
    src/util/future.h
    src/CustomPainter.cpp
    src/CustomPainter.h
    src/dse.cpp
)

set(DSE_MULTISAMPLE "" CACHE STRING "Enable multisampling with specified level")
set(DSE_THREADPOOL_TASKS_PER_IO "16" CACHE STRING "Task count per IO and UI handling")
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    option(DSE_SUBSYSTEM_WINDOWS "Use windows subsystem while linking executable binary" ON)
endif()

configure_file(config/dse_config.h.in config/dse_config.h)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(${CMAKE_PROJECT_NAME}
    ${SRCS}
    $<$<PLATFORM_ID:Windows>:${DSE_WIN32_SRCS}>
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    src
    swal/include
    include_gl
    ${PROJECT_BINARY_DIR}/config
)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${DSE_CXX_FLAGS})
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    $<$<BOOL:${DSE_SUBSYSTEM_WINDOWS}>:${DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS}>
)
target_link_libraries(${CMAKE_PROJECT_NAME} OpenGL::GL Threads::Threads dwmapi)
