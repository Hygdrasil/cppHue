set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_POSTION_INDEPENDENT_CODE ON)

set(LIB_BINARY ${CMAKE_PROJECT_NAME})

set(headers
    huebridge.hpp
    jsonnavi.hpp
    json_serialicer.hpp
    simplehttpclient.hpp)

set(sources
    huebridge.cpp
    jsonnavi.cpp
    json_serialicer.cpp
    simplehttpclient.cpp)
set(SOURCES ${headers} ${sources})
add_library(${LIB_BINARY} STATIC ${SOURCES})
target_include_directories( ${LIB_BINARY} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
