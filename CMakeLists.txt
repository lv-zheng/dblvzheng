cmake_minimum_required(VERSION 3.1)
project(dblvzheng)

set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(dblvzheng_SRC
	src/blockio.cpp
)

add_library(dblvzheng_obj OBJECT ${dblvzheng_SRC})
set_property(TARGET dblvzheng_obj PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(dblvzheng_shared SHARED $<TARGET_OBJECTS:dblvzheng_obj>)
add_library(dblvzheng_static STATIC $<TARGET_OBJECTS:dblvzheng_obj>)

set_target_properties(dblvzheng_shared PROPERTIES OUTPUT_NAME dblvzheng)
set_target_properties(dblvzheng_static PROPERTIES OUTPUT_NAME dblvzheng)

add_executable(test_block test/test_block.cpp)

target_link_libraries(test_block dblvzheng_static)
