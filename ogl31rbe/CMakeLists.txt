set(DSE_BUILT_IN_SHADERS
    gl31/draw.frag
    gl31/draw.vert
    gl31/post.frag
    gl31/post.vert
)

add_custom_command(OUTPUT
    generated/dse_shaders/gl31.cpp
COMMAND
    ${CMAKE_COMMAND}
    -DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/generated/dse_shaders/gl31
    -P gl31/pack_shaders_to_cpp.cmake ${DSE_BUILT_IN_SHADERS}
DEPENDS
    gl31/pack_shaders_to_cpp.cmake
    ${DSE_BUILT_IN_SHADERS}
BYPRODUCTS
    generated/dse_shaders/gl31.h
WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(ogl31shaders SOURCES gl31/pack_shaders_to_cpp.cmake ${DSE_BUILT_IN_SHADERS})

add_library(ogl31rbe SHARED)

target_include_directories(ogl31rbe PRIVATE
    ../include_gl
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(ogl31rbe PRIVATE core glbinding::glbinding Threads::Threads OpenGL::GL)

target_sources(ogl31rbe PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated/dse_shaders/gl31.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/generated/dse_shaders/gl31.h
    RenderOpenGL31.cpp
    RenderOpenGL31_impl.cpp
    RenderOpenGL31_impl.h
    gl31/MaterialInstance.cpp
    gl31/MaterialInstance.h
    gl31/MeshInstance.cpp
    gl31/MeshInstance.h
    gl31/ObjectInstance.cpp
    gl31/ObjectInstance.h
    gl31/RefCounted.cpp
    gl31/RefCounted.h
    gl31/Scene.cpp
    gl31/Scene.h
    gl31/TextureInstance.cpp
    gl31/TextureInstance.h
    gl31/binds.h
    glwrp/Buffer.h
    glwrp/Context.cpp
    glwrp/Context.h
    glwrp/FrameBuffer.h
    glwrp/ImportedFunction.h
    glwrp/Program.h
    glwrp/RenderBuffer.h
    glwrp/Sampler.h
    glwrp/Shader.h
    glwrp/Texture.h
    glwrp/TrvMvOnlyRes.h
    glwrp/VAO.h
    glwrp/gl.cpp
    glwrp/gl.h
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(ogl31rbe PRIVATE dwmapi swal::swal)
endif()
