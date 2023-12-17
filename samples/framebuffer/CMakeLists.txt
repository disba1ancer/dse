add_executable(framebuffer)

target_link_libraries(framebuffer PRIVATE core)
target_link_options(framebuffer PRIVATE
    $<$<BOOL:${DSE_SUBSYSTEM_WINDOWS}>:${DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS}>
)

target_sources(framebuffer PRIVATE
    main.cpp
)

install(TARGETS framebuffer)
