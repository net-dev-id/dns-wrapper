# --------------------------------------------------------------------------------
#                            Misc (no change needed).
# --------------------------------------------------------------------------------
# Have CMake parse the config file, generating the config header, with
# correct definitions. Here only used to make version number available to
# the source code. Include "version.h" (no .in suffix) in the source.

if(DEFINED ENV{GIT_BRANCH})
  set(GIT_BRANCH "$ENV{GIT_BRANCH}")
else()
  execute_process(
    COMMAND bash -c "git branch | sed -n 's/^\\* //p'"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

if(DEFINED ENV{GIT_HASH})
  set(GIT_HASH "$ENV{GIT_HASH}")
else()
  execute_process(
    COMMAND git --no-pager describe --always --abbrev=8 --dirty
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

if(DEFINED ENV{GIT_VERSION})
  set(GIT_VERSION "$ENV{GIT_VERSION}")
else()
  execute_process(
    COMMAND git --no-pager describe --tags --always --abbrev=8 --dirty
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

if(DEFINED ENV{GIT_DATE})
  set(GIT_DATE "$ENV{GIT_DATE}")
else()
  execute_process(
    COMMAND bash -c "git --no-pager show --date=short --format=\"%ai\" --name-only | head -n 1"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DATE
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

if(DEFINED ENV{GIT_TAG})
  set(GIT_TAG "$ENV{GIT_TAG}")
else()
  execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

if(DEFINED ENV{CI_ARCH})
  set(COMPILATION_ARCH "$ENV{CI_ARCH} (compiled on CI)")
else()
  execute_process(
    COMMAND uname -m
    OUTPUT_VARIABLE UNAME
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(COMPILATION_ARCH "${UNAME} (compiled locally)")
endif()

execute_process(
  COMMAND bash -c "${CMAKE_C_COMPILER} --version | head -n 1"
  OUTPUT_VARIABLE COMPILATION_CC
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
  "${PROJECT_SOURCE_DIR}/include/version.h.in"
  "${PROJECT_BINARY_DIR}/version.h"
)
# add the binary tree to the search path for include files
# so that we will find version.h
include_directories("${PROJECT_BINARY_DIR}")

# Ask CMake to output a compile_commands.json file for use with things like Vim YCM.
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
