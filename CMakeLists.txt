cmake_minimum_required(VERSION 3.1)
project(dblvzheng)

set(CMAKE_CXX_STANDARD 14)

# Find Boost

find_package(Boost 1.58 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)

set(dblvzheng_SRC
	dblvzheng/bgroup_query.cpp
	dblvzheng/blockio.cpp
	dblvzheng/database.cpp
	dblvzheng/index.cpp
	dblvzheng/hash.cpp
	dblvzheng/super.cpp
)

add_library(dblvzheng_obj OBJECT ${dblvzheng_SRC})
set_property(TARGET dblvzheng_obj PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(dblvzheng_shared SHARED $<TARGET_OBJECTS:dblvzheng_obj>)
add_library(dblvzheng_static STATIC $<TARGET_OBJECTS:dblvzheng_obj>)

set_target_properties(dblvzheng_shared PROPERTIES OUTPUT_NAME dblvzheng)
set_target_properties(dblvzheng_static PROPERTIES OUTPUT_NAME dblvzheng)

add_executable(test_bgroup_r test/test_bgroup_r.cpp)
add_executable(test_bgroup_w test/test_bgroup_w.cpp)
add_executable(test_block test/test_block.cpp)
add_executable(test_db test/test_db.cpp)
add_executable(test_hash test/test_hash.cpp)
add_executable(test_insert test/test_insert.cpp)
add_executable(test_mkfs test/test_mkfs.cpp)
add_executable(test_query test/test_query.cpp)
add_executable(test_remove test/test_remove.cpp)
add_executable(test_super_read test/test_super_read.cpp)

add_executable(benchmark_correctness benchmark/benchmark_correctness.cpp)
add_executable(benchmark_performance benchmark/benchmark_performance.cpp)

add_executable(gen_djbcolli test/gen_djbcolli.cpp)

target_link_libraries(test_bgroup_r dblvzheng_shared)
target_link_libraries(test_bgroup_w dblvzheng_shared)
target_link_libraries(test_block dblvzheng_shared)
target_link_libraries(test_db dblvzheng_shared)
target_link_libraries(test_hash dblvzheng_shared)
target_link_libraries(test_insert dblvzheng_shared)
target_link_libraries(test_mkfs dblvzheng_shared)
target_link_libraries(test_query dblvzheng_shared)
target_link_libraries(test_remove dblvzheng_shared)
target_link_libraries(test_super_read dblvzheng_shared)

target_link_libraries(benchmark_correctness dblvzheng_shared)
target_link_libraries(benchmark_performance dblvzheng_shared)
