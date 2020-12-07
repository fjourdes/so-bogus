################################################################################
#
# \file      cmake/FindMKL.cmake
# \author    J. Bakosi
# \copyright 2012-2015, Jozsef Bakosi, 2016, Los Alamos National Security, LLC.
# \brief     Find the Math Kernel Library from Intel
# \date      Thu 26 Jan 2017 02:05:50 PM MST
#
# \edit      F. Jourdes :
#            - added use of imported targets
#            - rely on MKL_ROOT_DIR CMake CACHE variable for library and header file lookup
#            - added option to select multithreading implementation ( OpenMP TBB )
#            - added option to select the indexing type ( 64bits or 32 bits )
#           See https://software.intel.com/en-us/articles/intel-mkl-link-line-advisor/
#           for all possible configurations
#
################################################################################

# Find the Math Kernel Library from Intel
#
#  MKL_FOUND - System has MKL
#  MKL_INCLUDE_DIRS - MKL include files directories
#  MKL_LIBRARIES - The MKL libraries
#  MKL_INTERFACE_LIBRARY - MKL interface library
#  MKL_SEQUENTIAL_LAYER_LIBRARY - MKL sequential layer library
#  MKL_CORE_LIBRARY - MKL core library
#
#  MKL_ROOT_DIR - The root dir used to lookup library and header files 
#                 e.g "C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2019.3.203\windows\mkl"
#
#  Example usage:
#
#  find_package(MKL)
#  if(MKL_FOUND)
#    target_link_libraries(TARGET ${MKL_LIBRARIES})
#  endif()

cmake_minimum_required(VERSION 3.0)

if(MSVC)
    if( NOT CMAKE_CL_64 )
        message("FindMKL only support 64 bits configuration")
    endif()
endif()

# If already in cache, be silent
if (MKL_INCLUDE_DIRS AND MKL_LIBRARIES AND MKL_INTERFACE_LIBRARY AND
    MKL_SEQUENTIAL_LAYER_LIBRARY AND MKL_CORE_LIBRARY)
    set (MKL_FIND_QUIETLY TRUE)
endif()

set(MKL_DEFAULT_DIR $ENV{MKL_ROOT})
set(MKL_ROOT_DIR ${MKL_DEFAULT_DIR} CACHE PATH "Path to search for MKL" )
set(MKL_USE_LARGE_ARRAY_INDEXING FALSE CACHE BOOL "If TRUE use ILP64 instead of LP64 library. FALSE by default")
set(MKL_USE_OPENMP_THREADING FALSE CACHE BOOL "If True use OpenMP threading library")
set(MKL_USE_TBB_THREADING FALSE CACHE BOOL "If True use TBB threading library")
set(MKL_USE_STATIC_LIB FALSE CACHE BOOL "If True use MKL static libraries")

set(LIB_PREFIX "")
set(LIB_SUFFIX "")
set(MKL_OPENMP_LIB "")
set(MKL_DIRECT_CALL_FLAG "-DMKL_DIRECT_CALL_SEQ")
set(MKL_DIRECT_CALL_JIT_FLAG "-DMKL_DIRECT_CALL_JIT_SEQ")

set(MKL_USE_THREADING OFF)
if( MKL_USE_TBB_THREADING OR MKL_USE_OPENMP_THREADING)
    set(MKL_USE_THREADING ON)
    set(MKL_DIRECT_CALL_FLAG "-DMKL_DIRECT_CALL")
    set(MKL_DIRECT_CALL_JIT_FLAG "-DMKL_DIRECT_CALL_JIT")
endif()

if( MKL_USE_TBB_THREADING AND MKL_USE_OPENMP_THREADING)
    message(WARNING "MKL_USE_TBB_THREADING and MKL_USE_OPENMP_THREADING are both ON. Keep only MKL_USE_TBB_THREADING")
    set(MKL_USE_OPENMP_THREADING FALSE CACHE BOOL "" FORCE)
endif()


if( ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC )
    set(LIB_PREFIX "")
    set(DYNAMINC_LIB_SUFFIX "_dll.lib")
    set(STATIC_LIB_SUFFIX ".lib")
    if( MKL_USE_STATIC_LIB )
        set(LIB_SUFFIX ${STATIC_LIB_SUFFIX})
    else()
        set(LIB_SUFFIX ${DYNAMINC_LIB_SUFFIX})
    endif()
    set(MKL_OPENMP_LIB "libiomp5md.lib")
elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
    set(LIB_PREFIX "lib")
    set(DYNAMINC_LIB_SUFFIX ".so")
    set(STATIC_LIB_SUFFIX ".a")
    if( MKL_USE_STATIC_LIB )
        set(LIB_SUFFIX ${STATIC_LIB_SUFFIX})
    else()
        set(LIB_SUFFIX ${DYNAMINC_LIB_SUFFIX})
    endif()
    set(MKL_OPENMP_LIB "iomp5")
endif()

if( MKL_USE_LARGE_ARRAY_INDEXING )
    set(INT_LIB "${LIB_PREFIX}mkl_intel_ilp64${LIB_SUFFIX}")
    set(BLAS_LIB "${LIB_PREFIX}mkl_blas95_ilp64${STATIC_LIB_SUFFIX}")
    set(LAPACK_LIB "${LIB_PREFIX}mkl_lapack95_ilp64${STATIC_LIB_SUFFIX}")
else()
    set(INT_LIB "${LIB_PREFIX}mkl_intel_lp64${LIB_SUFFIX}")
    set(BLAS_LIB "${LIB_PREFIX}mkl_blas95_lp64${STATIC_LIB_SUFFIX}")
    set(LAPACK_LIB "${LIB_PREFIX}mkl_lapack95_lp64${STATIC_LIB_SUFFIX}")
endif()
set(SEQ_LIB "${LIB_PREFIX}mkl_sequential${LIB_SUFFIX}")
set(COR_LIB "${LIB_PREFIX}mkl_core${LIB_SUFFIX}")
if( MKL_USE_OPENMP_THREADING )
    set(THR_LIB "${LIB_PREFIX}mkl_intel_thread${LIB_SUFFIX}")
    set(THR_BASE_LIB ${MKL_OPENMP_LIB} )
elseif( MKL_USE_TBB_THREADING )
    set(THR_LIB "${LIB_PREFIX}mkl_tbb_thread${LIB_SUFFIX}" )
    set(THR_BASE_LIB "tbb")
endif()

find_path(MKL_INCLUDE_DIR NAMES mkl.h HINTS ${MKL_ROOT_DIR}/include)

find_library(MKL_INTERFACE_LIBRARY
    NAMES ${INT_LIB}
    PATHS ${MKL_ROOT_DIR}/lib
    ${MKL_ROOT_DIR}/lib/intel64
    ${MKL_ROOT_DIR}/mkl/lib/intel64
    NO_DEFAULT_PATH)

find_library(MKL_BLAS_LIBRARY
    NAMES ${BLAS_LIB}
    PATHS ${MKL_ROOT_DIR}/lib
    ${MKL_ROOT_DIR}/lib/intel64
    ${MKL_ROOT_DIR}/mkl/lib/intel64
    NO_DEFAULT_PATH)

find_library(MKL_LAPACK_LIBRARY
    NAMES ${LAPACK_LIB}
    PATHS ${MKL_ROOT_DIR}/lib
    ${MKL_ROOT_DIR}/lib/intel64
    ${MKL_ROOT_DIR}/mkl/lib/intel64
    NO_DEFAULT_PATH)

find_library(MKL_SEQUENTIAL_LAYER_LIBRARY
    NAMES ${SEQ_LIB}
    PATHS ${MKL_ROOT_DIR}/lib
    ${MKL_ROOT_DIR}/lib/intel64
    ${MKL_ROOT_DIR}/mkl/lib/intel64
    NO_DEFAULT_PATH)

find_library(MKL_CORE_LIBRARY
    NAMES ${COR_LIB}
    PATHS ${MKL_ROOT_DIR}/lib
    ${MKL_ROOT_DIR}/lib/intel64
    ${MKL_ROOT_DIR}/mkl/lib/intel64
    NO_DEFAULT_PATH)

set(MKL_INCLUDE_DIRS ${MKL_INCLUDE_DIR})
set(MKL_LIBRARIES ${MKL_INTERFACE_LIBRARY} ${MKL_SEQUENTIAL_LAYER_LIBRARY} ${MKL_CORE_LIBRARY})

if( MKL_USE_THREADING )
    find_library(MKL_THREAD_LIBRARY
        NAMES ${THR_LIB}
        PATHS ${MKL_ROOT_DIR}/lib
        ${MKL_ROOT_DIR}/lib/intel64
        ${MKL_ROOT_DIR}/mkl/lib/intel64
        NO_DEFAULT_PATH)
    list(APPEND MKL_LIBRARIES ${MKL_THREAD_LIBRARY} )

    find_library(MKL_THREAD_BASE_LIBRARY
        NAMES ${THR_BASE_LIB}
        PATHS ${MKL_ROOT_DIR}/lib
        ${MKL_ROOT_DIR}/lib/intel64
        ${MKL_ROOT_DIR}/mkl/lib/intel64
        ${MKL_ROOT_DIR}/../tbb/lib/intel64/vc_mt
        ${MKL_ROOT_DIR}/../compiler/lib/intel64

        NO_DEFAULT_PATH)
    list(APPEND MKL_LIBRARIES ${MKL_THREAD_BASE_LIBRARY} )
endif()

# Handle the QUIETLY and REQUIRED arguments and set MKL_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
if(MKL_USE_THREADING)
    find_package_handle_standard_args(MKL REQUIRED_VARS 
        MKL_INCLUDE_DIRS MKL_INTERFACE_LIBRARY MKL_BLAS_LIBRARY MKL_LAPACK_LIBRARY MKL_SEQUENTIAL_LAYER_LIBRARY MKL_CORE_LIBRARY MKL_THREAD_BASE_LIBRARY MKL_THREAD_LIBRARY )
else()
    find_package_handle_standard_args(MKL REQUIRED_VARS 
        MKL_INCLUDE_DIRS MKL_INTERFACE_LIBRARY MKL_BLAS_LIBRARY MKL_LAPACK_LIBRARY MKL_SEQUENTIAL_LAYER_LIBRARY MKL_CORE_LIBRARY)
endif()

# Create imported targets
if(MKL_FOUND)

    if( NOT TARGET mkl_requirements ) #  dummy target to convey usage requirements for the rest of the MKL targets
        add_library(mkl_requirements INTERFACE)
        if(MKL_USE_LARGE_ARRAY_INDEXING)
            target_compile_options(mkl_requirements INTERFACE "-DMKL_ILP64")
        endif()
        target_compile_options(mkl_requirements INTERFACE "$<$<CONFIG:Release>:${MKL_DIRECT_CALL_FLAG}>")
        target_compile_options(mkl_requirements INTERFACE "$<$<CONFIG:Release>:${MKL_DIRECT_CALL_JIT_FLAG}>")
        target_link_libraries(mkl_requirements INTERFACE "$<$<CXX_COMPILER_ID:GNU>:-m -dl>" )
        if(MKL_USE_THREADING)
            target_link_libraries(mkl_requirements INTERFACE "$<$<CXX_COMPILER_ID:GNU>:-pthread>" )
        endif()
    endif()

    if(NOT TARGET MKL::blas)
        add_library( MKL::blas STATIC IMPORTED)
        set_target_properties( MKL::blas PROPERTIES IMPORTED_LOCATION ${MKL_BLAS_LIBRARY})
    endif()

    if(NOT TARGET MKL::lapack)
        add_library( MKL::lapack STATIC IMPORTED)
        set_target_properties( MKL::lapack PROPERTIES IMPORTED_LOCATION ${MKL_LAPACK_LIBRARY})
    endif()

    # For some unknown reason when setting SHARED or STATIC the IMPORTED_LOCATION gets rigged (maybe Fastbuild generator related ?)
    if(NOT TARGET MKL::interface )
        add_library( MKL::interface UNKNOWN IMPORTED )
        set_target_properties( MKL::interface PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
        set_target_properties( MKL::interface PROPERTIES IMPORTED_LOCATION ${MKL_INTERFACE_LIBRARY} )
        target_link_libraries(MKL::interface INTERFACE mkl_requirements)
    endif()

    if(NOT TARGET MKL::sequential )
        add_library( MKL::sequential UNKNOWN IMPORTED )
        set_target_properties( MKL::sequential PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
        set_target_properties( MKL::sequential PROPERTIES IMPORTED_LOCATION ${MKL_SEQUENTIAL_LAYER_LIBRARY} )
        target_link_libraries( MKL::sequential INTERFACE mkl_requirements)
    endif()

    if(NOT TARGET MKL::core )
        add_library( MKL::core UNKNOWN IMPORTED )
        set_target_properties( MKL::core PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
        set_target_properties( MKL::core PROPERTIES IMPORTED_LOCATION ${MKL_CORE_LIBRARY} )
        target_link_libraries(MKL::core INTERFACE mkl_requirements)
    endif()

    set(MKL_LIBRARIES MKL::interface MKL::sequential MKL::core )

    if( MKL_USE_THREADING )
        add_library( MKL::threading_base UNKNOWN IMPORTED )
        set_target_properties( MKL::threading_base PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
        set_target_properties( MKL::threading_base PROPERTIES IMPORTED_LOCATION ${MKL_THREAD_BASE_LIBRARY} )
        target_link_libraries(MKL::threading_base INTERFACE mkl_requirements)

        add_library( MKL::threading UNKNOWN IMPORTED )
        set_target_properties( MKL::threading PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
        set_target_properties( MKL::threading PROPERTIES IMPORTED_LOCATION ${MKL_THREAD_LIBRARY} )
        target_link_libraries(MKL::threading INTERFACE mkl_requirements)

        list(APPEND MKL_LIBRARIES MKL::threading MKL::threading_base)
    endif()
endif()

# find_package_handle_standard_args( MKL DEFAULT_MSG
# MKL_INCLUDE_DIRS MKL_INTERFACE_LIBRARY MKL_SEQUENTIAL_LAYER_LIBRARY MKL_CORE_LIBRARY )

mark_as_advanced(MKL_INCLUDE_DIRS MKL_LIBRARIES MKL_INTERFACE_LIBRARY MKL_SEQUENTIAL_LAYER_LIBRARY MKL_CORE_LIBRARY)
