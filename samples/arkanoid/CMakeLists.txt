add_executable(arkanoid)

target_link_libraries(arkanoid PRIVATE core)
target_link_options(arkanoid PRIVATE
    $<$<BOOL:${DSE_SUBSYSTEM_WINDOWS}>:${DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS}>
)

target_sources(arkanoid PRIVATE
    main.cpp
)

install(TARGETS arkanoid)
