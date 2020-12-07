project(Apps)


find_package(Bogus REQUIRED)

if( BOGUS_WITH_BOOST_SERIALIZATION )
    add_executable (LoadFrictionProblem LoadFrictionProblem.cpp)
    target_link_libraries (LoadFrictionProblem Bogus::bogus )
endif()

find_package(FCLib)
if( FCLIB_FOUND )
        add_executable (FCLibLoader FCLibLoader.cpp)
        target_link_libraries (FCLibLoader Bogus::bogus ${FCLib_LIBRARIES} )
endif()

