cmake_minimum_required(VERSION 3.24)
project(dse
    LANGUAGES CXX
    VERSION 0.0.1.0
    DESCRIPTION "disba1ancer's engine"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_WIN32_WINNT=0x0A00)

include(FetchContent)
FetchContent_Declare(
    swal
    GIT_REPOSITORY https://github.com/disba1ancer/swal.git
    GIT_TAG master
)
FetchContent_Declare(
    glbinding
    GIT_REPOSITORY https://github.com/cginternals/glbinding.git
    GIT_TAG 28d32d9bbc72aedf815f18113b0bd3aa7b354108
    FIND_PACKAGE_ARGS
)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

FetchContent_MakeAvailable(swal glbinding)
#unset(TEMP_EXTERNALS)

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
    src/core/ThreadPool_win32.cpp
    src/core/ThreadPool_win32.h
    src/core/errors_win32.cpp
    src/core/errors_win32.h
    src/os/PaintEventData_win32.h
    src/os/WindowData_win32.h
    src/os/WindowEventData_win32.h
    src/os/Window_win32.cpp
    src/os/Window_win32.h
    src/os/win32.h
)

set (DSE_CORE_SRCS
    ${PROJECT_BINARY_DIR}/generated/dse_shaders/gl31.cpp
    ${PROJECT_BINARY_DIR}/generated/dse_shaders/gl31.h
    src/core/File.cpp
    src/core/File.h
    src/core/ThreadPool.cpp
    src/core/ThreadPool.h
    src/core/errors.cpp
    src/core/errors.h
    src/math/constants.h
    src/math/mat.h
    src/math/qmath.h
    src/math/vec.h
    src/math/vmath.h
    src/notifier/notifier.h
    src/os/KeyboardKeyState.h
    src/os/Window.cpp
    src/os/Window.h
    src/os/WindowShowCommand.h
    src/os/mcursor.cpp
    src/os/mcursor.h
    src/renders/RenderOpenGL31.cpp
    src/renders/RenderOpenGL31.h
    src/renders/RenderOpenGL31_impl.cpp
    src/renders/RenderOpenGL31_impl.h
    src/renders/gl31/MaterialInstance.cpp
    src/renders/gl31/MaterialInstance.h
    src/renders/gl31/MeshInstance.cpp
    src/renders/gl31/MeshInstance.h
    src/renders/gl31/ObjectInstance.cpp
    src/renders/gl31/ObjectInstance.h
    src/renders/gl31/RefCounted.cpp
    src/renders/gl31/RefCounted.h
    src/renders/gl31/Scene.cpp
    src/renders/gl31/Scene.h
    src/renders/gl31/TextureInstance.cpp
    src/renders/gl31/TextureInstance.h
    src/renders/gl31/binds.h
    src/renders/glwrp/Buffer.h
    src/renders/glwrp/Context.cpp
    src/renders/glwrp/Context.h
    src/renders/glwrp/FrameBuffer.h
    src/renders/glwrp/ImportedFunction.h
    src/renders/glwrp/Program.h
    src/renders/glwrp/RenderBuffer.h
    src/renders/glwrp/Sampler.h
    src/renders/glwrp/Shader.h
    src/renders/glwrp/Texture.h
    src/renders/glwrp/TrvMvOnlyRes.h
    src/renders/glwrp/VAO.h
    src/renders/glwrp/gl.cpp
    src/renders/glwrp/gl.h
    src/scn/BasicBitmapLoader.cpp
    src/scn/BasicBitmapLoader.h
    src/scn/Camera.cpp
    src/scn/Camera.h
    src/scn/Cube.cpp
    src/scn/Cube.h
    src/scn/IMesh.h
    src/scn/ITextureDataProvider.h
    src/scn/Material.cpp
    src/scn/Material.h
    src/scn/Object.cpp
    src/scn/Object.h
    src/scn/Scene.cpp
    src/scn/Scene.h
    src/util/PoolAllocator.h
    src/util/ProxyContainer.h
    src/util/ProxyIterator.h
    src/util/TagInvoke.h
    src/util/TaskState.h
    src/util/access.h
    src/util/coroutine.h
    src/util/emptylock.h
    src/util/enum_bitwise.h
    src/util/execution.h
    src/util/functional.h
    src/util/impl_exec/base.h
    src/util/impl_exec/start_detached.h
    src/util/impl_exec/then.h
    src/util/impl_exec/wait.h
    src/util/pimpl.h
    src/util/scope_exit.h
    src/util/spinlock.h
    src/util/unlock_guard.h
)

set(DSE_BUILT_IN_SHADERS
    src/renders/gl31/draw.frag
    src/renders/gl31/draw.vert
    src/renders/gl31/post.frag
    src/renders/gl31/post.vert
)

add_custom_command(OUTPUT
    generated/dse_shaders/gl31.cpp
COMMAND
    ${CMAKE_COMMAND}
    -DOUTPUT=${PROJECT_BINARY_DIR}/generated/dse_shaders/gl31
    -P src/renders/gl31/pack_shaders_to_cpp.cmake ${DSE_BUILT_IN_SHADERS}
DEPENDS
    src/renders/gl31/pack_shaders_to_cpp.cmake
    ${DSE_BUILT_IN_SHADERS}
BYPRODUCTS
    generated/dse_shaders/gl31.h
WORKING_DIRECTORY
    ${PROJECT_SOURCE_DIR}
)

add_custom_target(shaders SOURCES src/renders/gl31/pack_shaders_to_cpp.cmake ${DSE_BUILT_IN_SHADERS})

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
set(DSE_RENDER_GC_OBJECTS_PER_FRAME "16" CACHE STRING "Object count wil be checked for deletion every frame")
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    option(DSE_SUBSYSTEM_WINDOWS "Use windows subsystem while linking executable binary" ON)
endif()

configure_file(config/dse_config.h.in generated/dse_config.h)

# Target  core
add_library(core OBJECT
    ${DSE_CORE_SRCS}
    $<$<PLATFORM_ID:Windows>:${DSE_CORE_WIN32_SRCS}>
)
target_include_directories(core PRIVATE
    include_gl
    ${PROJECT_BINARY_DIR}/generated
)
target_include_directories(core PUBLIC
    src
)
target_compile_options(core PRIVATE ${DSE_CXX_FLAGS})
target_link_libraries(core OpenGL::GL Threads::Threads glbinding::glbinding)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(core dwmapi swal::swal)
endif()

# Target  demo
add_executable(demo
    ${DSE_DEMO_SRCS}
    $<$<PLATFORM_ID:Windows>:${DSE_DEMO_WIN32_SRCS}>
)
target_include_directories(demo PRIVATE
    ${PROJECT_BINARY_DIR}/generated
)
target_compile_options(demo PRIVATE ${DSE_CXX_FLAGS})
target_link_options(demo PRIVATE
    $<$<BOOL:${DSE_SUBSYSTEM_WINDOWS}>:${DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS}>
)
target_link_libraries(demo core)
