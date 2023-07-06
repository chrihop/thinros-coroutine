cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROTOTHREAD_INC_DIR})

link_libraries(
    protothread
)


add_library(thinros_coroutine
    STATIC
    coroutine.c)

