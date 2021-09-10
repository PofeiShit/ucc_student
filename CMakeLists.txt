cmake_minimum_required(VERSION 3.5)

project(ucc_student)

option(BUILD_LLT OFF)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(UCL_DIR ${ROOT_DIR}/ucl)
set(GTEST_DIR ${ROOT_DIR}/googletest)

if (BUILD_LLT MATCHES ON)
    set(CMAKE_CXX_STANDARD 11)
    add_subdirectory(googletest)
endif()

add_subdirectory(ucl)



