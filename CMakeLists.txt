cmake_minimum_required(VERSION 3.19)
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
    $<$<BOOL:${DSE_CXX_MSVC}>:/W3>
)
set(DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS
    $<$<CXX_COMPILER_ID:GNU>:-mwindows>
    $<$<CXX_COMPILER_ID:MSVC>:/entry:mainCRTStartup;/subsystem:windows>
)

set(DSE_CORE_WIN32_SRCS
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
)

set (DSE_CORE_SRCS
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
    src/renders/glwrp/Buffer.h
    src/renders/glwrp/Context.cpp
    src/renders/glwrp/Context.h
    src/renders/glwrp/FrameBuffer.h
    src/renders/glwrp/gl.cpp
    src/renders/glwrp/gl.h
    src/renders/glwrp/ImportedFunction.h
    src/renders/glwrp/Program.h
    src/renders/glwrp/RenderBuffer.h
    src/renders/glwrp/Shader.h
    src/renders/glwrp/Texture.h
    src/renders/glwrp/TrvMvOnlyRes.h
    src/renders/glwrp/VAO.h
    src/renders/gl31/binds.h
    src/renders/gl31/MeshInstance.cpp
    src/renders/gl31/MeshInstance.h
    src/renders/gl31/ObjectInstance.cpp
    src/renders/gl31/ObjectInstance.h
    src/renders/gl31/Scene.cpp
    src/renders/gl31/Scene.h
    src/renders/gl31/shaders.cpp
    src/renders/gl31/shaders.h
    src/renders/RenderOpenGL31.cpp
    src/renders/RenderOpenGL31.h
    src/renders/RenderOpenGL31_impl.cpp
    src/renders/RenderOpenGL31_impl.h
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
)

set(DSE_DEMO_WIN32_SRCS
    rc_win32/icon.ico
    rc_win32/manifest.xml
    rc_win32/res.rc
)

set (DSE_DEMO_SRCS
    src/dse.cpp
    src/CustomPainter.cpp
    src/CustomPainter.h
)

set(DSE_MULTISAMPLE "" CACHE STRING "Enable multisampling with specified level")
set(DSE_THREADPOOL_TASKS_PER_IO "16" CACHE STRING "Task count per IO and UI handling")
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    option(DSE_SUBSYSTEM_WINDOWS "Use windows subsystem while linking executable binary" ON)
endif()

configure_file(config/dse_config.h.in config/dse_config.h)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
add_subdirectory(swal)

# Target  core
add_library(core OBJECT
    ${DSE_CORE_SRCS}
    $<$<PLATFORM_ID:Windows>:${DSE_CORE_WIN32_SRCS}>
)
target_include_directories(core PRIVATE
    include_gl
    ${PROJECT_BINARY_DIR}/config
)
target_include_directories(core PUBLIC
    src
)
target_compile_options(core PRIVATE ${DSE_CXX_FLAGS})
target_link_libraries(core OpenGL::GL Threads::Threads)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(core dwmapi swal::swal)
endif()

# Target  demo
add_executable(demo
    ${DSE_DEMO_SRCS}
    $<$<PLATFORM_ID:Windows>:${DSE_DEMO_WIN32_SRCS}>
)
target_include_directories(demo PRIVATE
    ${PROJECT_BINARY_DIR}/config
)
target_compile_options(demo PRIVATE ${DSE_CXX_FLAGS})
target_link_options(demo PRIVATE
    $<$<BOOL:${DSE_SUBSYSTEM_WINDOWS}>:${DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS}>
)
target_link_libraries(demo core)
