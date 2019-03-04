cmake_minimum_required(VERSION 2.8.2)

if( TESTS_NAMES )
    SET( SRC_LIST ${TESTS_NAMES} )
    MESSAGE( STATUS "Building tests: " ${SRC_LIST} )
else()
    execute_process( COMMAND ../updateCMakeSources.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    file(STRINGS CMakeSources.txt SRC_LIST)
    MESSAGE( STATUS "Building all tests" )
endif()

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)

add_subdirectory(googletest-release-1.8.1)

SET( GTEST_LINK_LIBRARIES gtest gtest_main )
if(UNIX)
  SET( GTEST_LINK_LIBRARIES ${GTEST_LINK_LIBRARIES} pthread )
endif()

include_directories( ../src ${source_dir}/include)

add_executable(test${CMAKE_PROJECT_NAME} ${SRC_LIST})
add_test( ${CMAKE_PROJECT_NAME} test${CMAKE_PROJECT_NAME} )

target_link_libraries(test${CMAKE_PROJECT_NAME} ${EXTERNAL_LIBRARIES} ${GTEST_LINK_LIBRARIES} )
