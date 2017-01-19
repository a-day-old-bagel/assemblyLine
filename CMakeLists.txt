cmake_minimum_required( VERSION 3.3 )
set( CMAKE_DISABLE_SOURCE_CHANGES ON )
set( CMAKE_DISABLE_IN_SOURCE_BUILD ON )
project( assemblyLine )

option( ASLIN_BUILD_TESTS "Build tests" ON )

if ( ASLIN_BUILD_TESTS )
    include_directories( "./inc" )
    add_subdirectory( "./src" )
endif ()