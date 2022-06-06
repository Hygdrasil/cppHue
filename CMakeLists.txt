cmake_minimum_required(VERSION 3.23)
project(cppHue
        VERSION 0.0.1
        DESCRIPTION "simple philips hue lib for esp32 and linux"
        LANGUAGES CXX
        )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_POSTION_INDEPENDENT_CODE ON)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
