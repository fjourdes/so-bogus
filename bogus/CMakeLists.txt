project(bogus)

OPTION( OPENMP "Use OpenMP" ON )
OPTION( BOOST_SERIALIZATION "Use boost::serialization" ON )
OPTION( MKL "Use mkl" OFF )

OPTION( WITH_2D "Build lib with 2d support" ON )
OPTION( WITH_3D "Build lib with 3d support" ON )
OPTION( WITH_DYNAMIC "Build lib with dynamic dim support" OFF )


execute_process( COMMAND ../updateCMakeSources.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
file(STRINGS CMakeSources.txt SRC_LIST)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -S --verbose-asm " )
find_package(Eigen3 REQUIRED)

add_library(bogus SHARED ${SRC_LIST})
# Alias target for project which includes directly bogus in their build tree
add_library(Bogus::bogus ALIAS bogus)

target_link_libraries(bogus PUBLIC Eigen3::Eigen)
target_include_directories(bogus PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../> )

if(OPENMP)
    find_package(OpenMP REQUIRED)
    target_link_libraries(bogus PUBLIC OpenMP::OpenMP_CXX)
    target_compile_options(bogus PUBLIC -DBOGUS_WITH_OPENMP)

endif()

if( MKL )
    find_package( MKL REQUIRED)
    target_link_libraries(bogus PUBLIC ${MKL_LIBRARIES})
    target_compile_options(bogus PUBLIC -DBOGUS_WITH_MKL)
endif()

if( BOOST_SERIALIZATION )
    find_package(Boost COMPONENTS serialization REQUIRED )
    target_link_libraries(bogus PUBLIC Boost::serialization Boost::disable_autolinking Boost::dynamic_linking)
    # target_compile_options(bogus PUBLIC -DBOOST_ALL_NO_LIB)
    # target_compile_options(bogus PUBLIC -DBOOST_ALL_DYN_LINK)
    target_compile_options(bogus PUBLIC -DBOGUS_WITH_BOOST_SERIALIZATION)
    target_compile_options(bogus PRIVATE -DBUILD_BOGUS)
endif()

if( WITH_2D )
    target_compile_options(bogus PUBLIC -DBOGUS_INSTANTIATE_2D_SOC )
endif()
if( WITH_3D )
    target_compile_options(bogus PUBLIC -DBOGUS_INSTANTIATE_3D_SOC )
endif()
if( WITH_DYNAMIC )
    target_compile_options(bogus PUBLIC -DBOGUS_INSTANTIATE_DYNAMIC_SOC )
endif()

if( MSVC )
    target_compile_options(bogus PUBLIC -DBOGUS_DONT_USE_BUILTIN_ATOMICS)
    target_compile_options(bogus PUBLIC /bigobj)
    target_compile_options(bogus PRIVATE /wd4305)
endif()

foreach(item ${SRC_LIST})
    string(REGEX MATCH "^Interfaces/.+\\.hpp$" item ${item})
    if(item)
        list(APPEND INTERFACES ${item})
    endif(item)
endforeach()

include(GNUInstallDirs)

install(TARGETS bogus
        EXPORT BogusTargets
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        RUNTIME DESTINATION  "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION  "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION  "${CMAKE_INSTALL_LIBDIR}"
)

install( DIRECTORY Core DESTINATION include/bogus )
install( DIRECTORY Extra DESTINATION include/bogus )
install( FILES ${INTERFACES} DESTINATION include/bogus/Interfaces )

include(CMakePackageConfigHelpers)

configure_package_config_file(BogusConfig.cmake.in
    ${CMAKE_BINARY_DIR}/Bogus/BogusConfig.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Bogus"
)

install(FILES ${CMAKE_BINARY_DIR}/Bogus/BogusConfig.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Bogus")

install( EXPORT BogusTargets
        NAMESPACE "Bogus::"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Bogus")
