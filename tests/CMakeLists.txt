project(bogus_test)
cmake_minimum_required(VERSION 2.8.2)


find_package(Bogus REQUIRED)

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


add_executable(bogus_test ${SRC_LIST})
target_link_libraries(bogus_test PUBLIC Bogus::bogus ${GTEST_LINK_LIBRARIES} )
target_include_directories(bogus_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src/)
target_compile_options(bogus_test PUBLIC -DBOOST_ALL_NO_LIB)
target_compile_options(bogus_test PUBLIC -DBOOST_ALL_DYN_LINK)

add_test( NAME bogus_test COMMAND bogus_test )
