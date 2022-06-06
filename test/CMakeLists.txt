cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_POSTION_INDEPENDENT_CODE ON)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.hpp *.cpp)

set(pcTest PcTester)
set(pcTestSrc pcTest.cpp)

add_executable(${pcTest} ${pcTestSrc})

find_package( GTest REQUIRED )      
include_directories( ${GTEST_INCLUDE_DIRS} )
target_link_libraries( ${pcTest} LINK_PUBLIC ${GTEST_LIBRARIES} ${CMAKE_PROJECT_NAME})

enable_testing()
add_test(
   NAME ${pcTest}}
   COMMAND ${pcTest}
)
