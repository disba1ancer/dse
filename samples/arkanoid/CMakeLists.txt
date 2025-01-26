add_executable(arkanoid)

target_link_libraries(arkanoid PRIVATE core)
target_link_options(arkanoid PRIVATE
    $<$<BOOL:${DSE_SUBSYSTEM_WINDOWS}>:${DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS}>
)

target_sources(arkanoid PRIVATE
    main.cpp
)

# if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
# target_sources(arkanoid PRIVATE
#    ../demo/rc_win32/icon.ico
#    ../demo/rc_win32/manifest.xml
#    ../demo/rc_win32/res.rc
# )
# endif()

install(TARGETS arkanoid)
