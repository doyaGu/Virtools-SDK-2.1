cmake_minimum_required(VERSION 3.12)

project(VirtoolsSDK LANGUAGES CXX VERSION 2.1.0)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED NO)
set(CMAKE_CXX_EXTENSIONS YES)

if (NOT WIN32)
    message(FATAL_ERROR "Only support Windows.")
endif ()

# Create imported targets: CK2 and VxMath
foreach (name IN ITEMS CK2 VxMath)
    add_library(${name} UNKNOWN IMPORTED)
    add_library(VirtoolsSDK::${name} ALIAS ${name})
    set_target_properties(${name} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/Include"
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/Lib/${name}.lib"
            )
    # Disable strict const-qualification conformance for pointers initialized by using string literals
    target_compile_options(${name} INTERFACE
            $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:MSVC>:/Zc:strictStrings->>
            $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-fpermissive>>
            $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-Wno-write-strings>>
            )
endforeach ()