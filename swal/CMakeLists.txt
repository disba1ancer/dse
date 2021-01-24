cmake_minimum_required(VERSION 3.19)
project (swal
    LANGUAGES CXX
    VERSION 0.1
    DESCRIPTION "Simple WinAPI Abstraction Layer"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target  swal
add_library(swal INTERFACE
    include/swal/com.h
    include/swal/enum_bitwise.h
    include/swal/error.h
    include/swal/gdi.h
    include/swal/handle.h
    include/swal/strconv.h
    include/swal/window.h
    include/swal/zero_or_resource.h
    include/swal/win_headers.h
)
target_include_directories(swal INTERFACE
    include
)

add_library(swal::swal ALIAS swal)
