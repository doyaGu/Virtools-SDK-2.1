set(VIRTOOLS_SDK_VERSION 2.1.0.14)

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

set_and_check(VIRTOOLS_SDK_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/Include")
set_and_check(VIRTOOLS_SDK_LIBRARY_DIR "${PACKAGE_PREFIX_DIR}/Lib")

include("${CMAKE_CURRENT_LIST_DIR}/VirtoolsSDKTargets.cmake")