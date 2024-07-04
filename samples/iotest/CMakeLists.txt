add_executable(iotest)

target_link_libraries(iotest PRIVATE core)
target_link_options(iotest PRIVATE
    $<$<BOOL:${DSE_SUBSYSTEM_WINDOWS}>:${DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS}>
)

target_sources(iotest PRIVATE
    main.cpp
)

install(TARGETS iotest)
