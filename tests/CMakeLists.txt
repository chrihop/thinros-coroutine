cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include_directories(
    ${THINROS_COROUTINE_TOP_DIR}/src
    ${PROTOTHREAD_INC_DIR}
)

link_libraries(
    protothread
    thinros_coroutine
)

add_executable(main-protothread
    main-protothread.c
    ${PROTOTHREAD_INC_DIR}/pt.h
)

target_link_libraries(main-protothread
    PRIVATE
    pthread
)
