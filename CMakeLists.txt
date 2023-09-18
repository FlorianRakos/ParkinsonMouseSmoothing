cmake_minimum_required(VERSION 3.23)
project(RawMouseAnalyzer)

set(CMAKE_CXX_STANDARD 17) #was 14 before

add_executable(RawMouseAnalyzer main.cpp)

include_directories("libraries/rapidjson/include")

