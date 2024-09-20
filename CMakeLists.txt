# 
# Copyright (c) 2024 Neeraj Jakhar
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.



# This file specifies how the project should be built, using CMake.
# If you are unfamiliar with CMake, don't worry about all the details.
# The sections you might want to edit are marked as such, and
# the comments should hopefully make most of it clear.
#
# For many purposes, you may not need to change anything about this file.

cmake_minimum_required(VERSION 3.15)

# Set project name, version and laguages here. (change as needed)
# Version numbers are available by including "exampleConfig.h" in
# the source. See exampleConfig.h.in for some more details.
set(PROJECT_NAME dns_wrapper)
project(${PROJECT_NAME} VERSION 0.0.0.1 LANGUAGES C CXX)

# Options: Things you can set via commandline options to cmake (e.g. -DENABLE_LTO=[ON|OFF])
option(ENABLE_WARNINGS_SETTINGS "Allow target_set_warnings to add flags and defines.
                                 Set this to OFF if you want to provide your own warning parameters."  ON)
option(ENABLE_LTO "Enable link time optimization" ON)
option(ENABLE_DOCTESTS "Include tests in the library. Setting this to OFF will remove all doctest related code.
                        Tests in tests/*.cpp will still be enabled."  ON)

# Include stuff. No change needed.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)
include(CTest)
include(Doctest)
include(Documentation)
include(LTO)
include(Version)
include(Warnings)

# Check for LTO support.
find_lto(CXX)

if(MSVC)
  # As per
  # https://github.com/microsoft/vcpkg/discussions/22762
  add_definitions(-D_WIN32_WINNT=0x0602)
else()
  # As per
  # https://stackoverflow.com/questions/23137637/linker-error-while-linking-boost-log-tutorial-undefined-references
  add_definitions(-DBOOST_LOG_DYN_LINK=1)
endif(MSVC)

# --------------------------------------------------------------------------------
#                         Locate files (change as needed).
# --------------------------------------------------------------------------------
file(GLOB_RECURSE SOURCES src/*.cpp src/*.c src/bookkeeping/*.cpp src/dns/*.cpp src/rule/*.cpp)
file(GLOB_RECURSE TESTFILES tests/*.cpp)

if(WIN32)
  message(STATUS "Detected Windows")
  list(FILTER SOURCES EXCLUDE REGEX "src/unix")
elseif(UNIX)
  message(STATUS "Detected UNIX")
  list(FILTER SOURCES EXCLUDE REGEX "src/win32")
endif(WIN32)

set(LIBRARY_NAME engine) # Default name for the library built from src/*.cpp (change if you wish)
add_library(${LIBRARY_NAME} OBJECT ${SOURCES} ${SOURCES_OS})

# Lib needs its header files, and users of the library must also see these (PUBLIC). (No change needed)
target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# There's also (probably) doctests within the library, so we need to see this as well.
target_link_libraries(${LIBRARY_NAME} PUBLIC doctest)


# Set the compile options you want (change as needed).
target_set_warnings(${LIBRARY_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying)
# target_compile_options(${LIBRARY_NAME} ... )  # For setting manually.

# Add an executable for the file app/main.cpp.
# If you add more executables, copy these lines accordingly.
set(EXEC_NAME dns-wrapper)
add_executable(${EXEC_NAME} app/main.cpp) # Name of exec. and location of file.
target_link_libraries(${EXEC_NAME} ${LIBRARY_NAME}) # Link the executable to library (if it uses it).
target_set_warnings(${EXEC_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).
target_enable_lto(${EXEC_NAME} optimized) # enable link-time-optimization if available for non-debug configurations

if (WIN32)
  target_link_libraries(${EXEC_NAME} iphlpapi)
endif(WIN32)

################################################################################
## BOOST ##
###########
#set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost CONFIG REQUIRED COMPONENTS system program_options log thread log_setup)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${EXEC_NAME} ${Boost_LIBRARIES})
link_directories(${Boost_LIBRARY_DIRS})

#################################################################################


set_target_properties(
  ${LIBRARY_NAME} ${EXEC_NAME}
  PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT WIN32)
  # Set up tests (see tests/CMakeLists.txt).
  add_subdirectory(tests)
endif(NOT WIN32)

message(STATUS "")
message(STATUS "")
message(STATUS "${PROJECT_NAME} configuration summary:")
message(STATUS "")
message(STATUS "  CMake build type ................ : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build shared libs  .............. : ${BUILD_SHARED_LIBS}")
message(STATUS "  Build static libs  .............. : ${BUILD_STATIC_LIBS}")
message(STATUS "  Build example  .................. : ${BUILD_EXAMPLE}")
message(STATUS "  Build test ...................... : ${BUILD_TEST}")
message(STATUS "  Build c++ standard .............. : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build with boost::filesystem .... : ${USE_BOOST_FILESYSTEM}")
message(STATUS "")
message(STATUS " Dependencies:")
message(STATUS "  Boost version.................... : ${Boost_VERSION}")
message(STATUS "  Boost root....................... : ${Boost_DIR}")
message(STATUS "  Boost lib version................ : ${Boost_LIB_VERSION}")
message(STATUS "  Boost root....................... : ${BOOST_ROOT}")
message(STATUS "  Boost include dirs............... : ${Boost_INCLUDE_DIRS}")
message(STATUS "  Boost library dirs............... : ${Boost_LIBRARY_DIRS}")
message(STATUS "  Boost libraries.................. : ${Boost_LIBRARIES}")

message(STATUS "")
message(STATUS " Git Information")
message(STATUS "   - Branch: ${GIT_BRANCH}")
message(STATUS "   - Architecture: ${COMPILATION_ARCH}")
message(STATUS "   - Version: ${GIT_VERSION}")
message(STATUS "   - Tag: ${GIT_TAG}")
message(STATUS "   - Hash: ${GIT_HASH}")
message(STATUS "   - Commit date: ${GIT_DATE}")
message(STATUS "   - Compiler: ${COMPILATION_CC}")
message(STATUS "")
