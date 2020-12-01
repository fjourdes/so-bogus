
if( BOOST_SERIALIZATION )
                add_executable (LoadFrictionProblem LoadFrictionProblem.cpp)
                target_link_libraries (LoadFrictionProblem bogus )
endif()

find_package(FCLib)
if( FCLIB_FOUND )
        
        add_executable (FCLibLoader FCLibLoader.cpp)
        target_link_libraries (FCLibLoader bogus ${FCLib_LIBRARIES} )
endif()

