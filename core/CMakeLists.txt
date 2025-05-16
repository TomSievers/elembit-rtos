cmake_minimum_required(VERSION 3.10)

project(elembit-rtos)

cmake_policy(SET CMP0077 NEW)

option(ENABLE_AMP "Enable asymmetric multi-processing" OFF)
option(ENABLE_SMP "Enable symmetric multi-processing" OFF)
option(ENABLE_SCHED_RR "Enable round-robin scheduling" OFF)

add_library(${PROJECT_NAME} STATIC)

set(DEFINES "")

if (ENABLE_AMP)
    message(STATUS "Asymmetric multi-processing enabled")
    list(APPEND DEFINES "ASSYMMETRIC_MULTI_PROCESSING")
endif()

if (ENABLE_SMP OR ENABLE_AMP)
    message(STATUS "Multi-processing enabled")
    list(APPEND DEFINES "MULTI_PROCESSING")
endif()

if (ENABLE_SCHED_RR)
    message(STATUS "Round-robin scheduling enabled")
    list(APPEND DEFINES "RR_SCHEDULE")
else()
    message(STATUS "FIFO scheduling enabled")
    list(APPEND DEFINES "FIFO_SCHEDULE")
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC
    ${DEFINES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_compile_options(${PROJECT_NAME} PUBLIC
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)