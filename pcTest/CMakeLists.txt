cmake_minimum_required(VERSION 3.5)

project(pcTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pcTest json_serialicer.h json_serialicer.cpp jsonnavi.h jsonnavi.cpp simplehttpclient.h simplehttpclient.cpp huebridge.h huebridge.cpp main.cpp)

