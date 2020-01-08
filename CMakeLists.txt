list ( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake )
include ( GodotHelpers )

cmake_minimum_required ( VERSION 3.15 )
project ( godot_chess )

set ( CMAKE_CXX_STANDARD 20 )
set ( CMAKE_CXX_STANDARD_REQUIRED ON )
set ( CMAKE_CXX_EXTENSIONS OFF )
set ( CMAKE_POSITION_INDEPENDENT_CODE ON )

add_subdirectory ( vendor )
add_subdirectory ( src )
