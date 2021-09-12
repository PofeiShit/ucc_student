cmake_minimum_required(VERSION 3.5)

project(ucl)

include_directories(${UCL_DIR}/src ${GTEST_DIR}/googletest/include)

if (BUILD_LLT MATCHES ON)
    set(CMAKE_CXX_STANDARD 11)

    file(GLOB_RECURSE CFILES "${UCL_DIR}/src/*.c")
    SET_SOURCE_FILES_PROPERTIES(${CFILES} PROPERTIES LANGUAGE CXX )

    aux_source_directory(${UCL_DIR}/src SRC)
    list(REMOVE_ITEM SRC ${UCL_DIR}/src/ucl.c)
    message(INFO ${SRC})
    aux_source_directory(${UCL_DIR}/test_case TEST_CASE_SRC)
    add_executable(${PROJECT_NAME} ${TEST_CASE_SRC} ${SRC})
    set(GTEST_LIB gtest gtest_main)
    target_link_libraries(${PROJECT_NAME} ${GTEST_LIB})


else()
    aux_source_directory(${UCL_DIR}/src SRC)
    add_executable(${PROJECT_NAME} ${SRC})
endif()








