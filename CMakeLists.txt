cmake_minimum_required(VERSION 3.12)

project(VirtoolsSDK LANGUAGES CXX VERSION 2.1.0)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS ON)

if (NOT WIN32)
    message(WARNING "[VirtoolsSDK] Non-Windows platform detected. Targets will not be usable.")
endif ()

set(VIRTOOLS_SDK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Include")
set(VIRTOOLS_SDK_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Lib")

# Create imported targets: CK2 and VxMath
foreach (name IN ITEMS CK2 VxMath)
    add_library(${name} UNKNOWN IMPORTED)
    add_library(VirtoolsSDK::${name} ALIAS ${name})
    set_target_properties(${name} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${VIRTOOLS_SDK_INCLUDE_DIR}"
        IMPORTED_LOCATION "${VIRTOOLS_SDK_LIB_DIR}/${name}.lib"
    )
    # Disable strict const-qualification conformance for pointers initialized by using string literals
    target_compile_options(${name} INTERFACE
            $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:MSVC>:/Zc:strictStrings->>
            $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-fpermissive>>
            $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-Wno-write-strings>>
            )
endforeach ()

# Exported variables
set(VirtoolsSDK_INCLUDE_DIR "${VIRTOOLS_SDK_INCLUDE_DIR}" CACHE PATH "VirtoolsSDK include directory")
set(VirtoolsSDK_LIB_DIR "${VIRTOOLS_SDK_LIB_DIR}" CACHE PATH "VirtoolsSDK library directory")