add_executable(demo)

target_link_libraries(demo PRIVATE core ogl31video)
target_link_options(demo PRIVATE
    $<$<BOOL:${DSE_SUBSYSTEM_WINDOWS}>:${DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS}>
)

target_sources(demo PRIVATE
    dse.cpp
    CustomPainter.cpp
    CustomPainter.h
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
FetchContent_MakeAvailable(swal)
target_link_libraries(demo PRIVATE swal::swal)
target_sources(demo PRIVATE
    rc_win32/icon.ico
    rc_win32/manifest.xml
    rc_win32/res.rc
)
endif()
