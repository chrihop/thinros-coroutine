cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(thinros-coroutine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

# Options
option(THINROS_COROUTINE_BUILD_TESTS "Build tests" ON)
##


# Global variables
set (THINROS_COROUTINE_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

##

include(FetchContent)

# Fetch Protothread
FetchContent_Declare(
    protothread
    GIT_REPOSITORY https://github.com/zserge/pt.git
    GIT_TAG master
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

message(STATUS "Fetching Protothread ...")
FetchContent_MakeAvailable(protothread)
set (PROTOTHREAD_INC_DIR ${protothread_SOURCE_DIR})
add_library(protothread INTERFACE)
target_include_directories(protothread INTERFACE ${PROTOTHREAD_INC_DIR})
##

# Build the library
add_subdirectory(src)
##

# Build the tests
if (THINROS_COROUTINE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
