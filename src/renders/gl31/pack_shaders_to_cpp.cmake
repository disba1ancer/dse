set(DSE_SHADERS_H
"#ifndef DSE_GENERATED_SHADERS_HEADER
#define DSE_GENERATED_SHADERS_HEADER
namespace dse::renders::gl31 {
")
set(DSE_SHADERS_CPP "namespace dse::renders::gl31 {\n")
math(EXPR argc "${CMAKE_ARGC} - 1")
set(start ${argc})
foreach(i RANGE ${argc})
    if("${CMAKE_ARGV${i}}" STREQUAL "-P")
        set(start ${i})
        math(EXPR start "${start} + 2")
        break()
    endif()
endforeach()
foreach(i RANGE ${start} ${argc})
    file(READ ${CMAKE_ARGV${i}} cont)
    get_filename_component(name ${CMAKE_ARGV${i}} NAME)
    string(REPLACE . _ name ${name})
    string(APPEND DSE_SHADERS_H "extern char shader_${name}[];\n")
    string(APPEND DSE_SHADERS_CPP "char shader_${name}[] = R\"shader(" "${cont}" ")shader\";\n")
endforeach()
message("${i}")
string(APPEND DSE_SHADERS_H
"}
#endif
")
string(APPEND DSE_SHADERS_CPP "}\n")
file(WRITE ${OUTPUT}.h "${DSE_SHADERS_H}")
file(WRITE ${OUTPUT}.cpp "${DSE_SHADERS_CPP}")
