if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
add_executable(smdxtest)
FetchContent_MakeAvailable(swal)

target_link_libraries(smdxtest PRIVATE core swal::swal dxgi d3d11)
target_link_options(smdxtest PRIVATE
    $<$<BOOL:${DSE_SUBSYSTEM_WINDOWS}>:${DSE_SUBSYSTEM_WINDOWS_LINK_OPTIONS}>
)
target_compile_definitions(smdxtest PRIVATE UNICODE=1 _WIN32_WINNT=0x0A00)

target_sources(smdxtest PRIVATE
    main.cpp
)

install(TARGETS smdxtest)
endif()
