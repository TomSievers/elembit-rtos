target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/message_box.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mutex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/semaphore.c
    ${CMAKE_CURRENT_SOURCE_DIR}/scheduler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/heap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/waker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/round_robin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/fifo.c
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    -DROUND_ROBIN
)