cmake_minimum_required(VERSION 3.23)
project(RawMouseAnalyzer)

set(CMAKE_CXX_STANDARD 17) #was 14 before

add_executable(RawMouseAnalyzer getData.cpp)

add_executable(RawMouseSmoother smoothMouse.cpp)

include_directories("libraries/rapidjson/include")

