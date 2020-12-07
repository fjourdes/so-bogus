project(so-bogus)
cmake_minimum_required(VERSION 2.6)

OPTION( TESTS "Build tests" ON )
OPTION( APPS "Build apps" ON )
OPTION( LIB "Build lib" ON )


OPTION( STRICT    "Treat warnings as errors (-Werror)" OFF )
OPTION( FAST_MATH "Enable unsafe compiler optimizations " ON )

SET( SOBOGUS_ROOT ${CMAKE_CURRENT_SOURCE_DIR} )
SET( CMAKE_MODULE_PATH ${SOBOGUS_ROOT}/cmake )

# Build types and processor flags

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# FindMKL (build)
configure_file( ${SOBOGUS_ROOT}/cmake/FindMKL.cmake
                ${CMAKE_BINARY_DIR}/Bogus/FindMKL.cmake COPYONLY)

# FindMKL (installed)
install(FILES ${SOBOGUS_ROOT}/cmake/FindMKL.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Bogus/FindMKL.cmake )


# Require C++11 compiler
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Visual studio solution directories.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if((CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT))
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Prefix prepended to install directories" FORCE)
    message(STATUS "Setting default CMAKE_INSTALL_PREFIX to: ${CMAKE_INSTALL_PREFIX}")
endif()

if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  set (CLANG TRUE)
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  set (GCC TRUE)
endif()

if ( GCC OR CLANG )
				SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra " )
				SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes " ) #gcc 6 + eigen
				if( STRICT )
						SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror " )
				endif()
				if( FAST_MATH )
						SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math" )
				endif()

				if( GCC AND APPLE )
						SET( EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} stdc++ )
				endif()

				#Do not use -march=native on gcc < 4.3
				SET(HANDLE_MARCH_NATIVE TRUE)
				if (GCC)
								execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
								if ( NOT GCC_VERSION VERSION_GREATER 4.3 AND NOT GCC_VERSION VERSION_EQUAL 4.3 )
												SET(HANDLE_MARCH_NATIVE FALSE)
								endif()

				endif()

				if( HANDLE_MARCH_NATIVE )
								SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -march=native -DEIGEN_NO_DEBUG ")
				else()
								SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -msse -msse2 -DEIGEN_NO_DEBUG ")
				endif()
endif()

if( LIB )
		add_subdirectory( bogus )
endif()
if( TESTS )
				enable_testing()
				add_subdirectory( tests )
endif()
if( APPS )
				add_subdirectory( apps )
endif()
