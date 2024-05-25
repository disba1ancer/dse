find_package(Threads REQUIRED)

add_library(core STATIC)

target_link_libraries(core PUBLIC headers)
target_link_libraries(core PRIVATE Threads::Threads)

target_compile_definitions(core PRIVATE DSE_CORE_EXPORT=1)

target_include_directories(core PUBLIC
    include
)

target_sources(core PUBLIC
    include/dse/core/BasicBitmapLoader.h
    include/dse/core/Camera.h
    include/dse/core/Cube.h
    include/dse/core/File.h
    include/dse/core/FrameBuffer.h
    include/dse/core/IMesh.h
    include/dse/core/IOContext.h
    include/dse/core/ITextureDataProvider.h
    include/dse/core/Image.h
    include/dse/core/ImageManipulator.h
    include/dse/core/KeyboardKeyState.h
    include/dse/core/Material.h
    include/dse/core/Object.h
    include/dse/core/Scene.h
    include/dse/core/Sphere.h
    include/dse/core/ThreadPool.h
    include/dse/core/UILoop.h
    include/dse/core/Window.h
    include/dse/core/WindowShowCommand.h
    include/dse/core/detail/impexp.h
    include/dse/core/detail/staticmod.h
    include/dse/core/errors.h
    include/dse/core/mcursor.h
    include/dse/core/modloader.h
    include/dse/core/status.h
    include/dse/core/statusdef.h
    include/dse/renders/RenderOpenGL31.h
)

target_sources(core PRIVATE
    BasicBitmapLoader.cpp
    Camera.cpp
    Cube.cpp
    File.cpp
    FrameBuffer.cpp
    IOContext.cpp
    Image.cpp
    ImageManipulator.cpp
    Material.cpp
    Object.cpp
    Scene.cpp
    Sphere.cpp
    ThreadPool.cpp
    UILoop.cpp
    Window.cpp
    errors.cpp
    mcursor.cpp
    modloader.cpp
    status.cpp
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
FetchContent_MakeAvailable(swal)
target_link_libraries(core PRIVATE swal::swal)
target_sources(core PUBLIC
    include/dse/core/PaintEventData_win32.h
    include/dse/core/WindowData_win32.h
    include/dse/core/WindowEventData_win32.h
    include/dse/core/win32.h
)

target_sources(core PRIVATE
    File_win32.cpp
    File_win32.h
    FrameBuffer_win32.cpp
    FrameBuffer_win32.h
    IOContext_win32.cpp
    IOContext_win32.h
    ThreadPool_win32.cpp
    ThreadPool_win32.h
    UILoop_win32.cpp
    UILoop_win32.h
    Window_win32.cpp
    Window_win32.h
    errors_win32.cpp
    errors_win32.h
    status_win32.cpp
    status_win32.h
)
endif()

install(TARGETS core PUBLIC_HEADER)
#install(EXPORT core NAMESPACE dse DESTINATION ./core)
