cmake_minimum_required(VERSION 3.10)

project(elembit-rtos)

set(ENABLE_AMP ON)
set(ENABLE_SMP ON)
set(ENABLE_SCHED_RR ON)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)