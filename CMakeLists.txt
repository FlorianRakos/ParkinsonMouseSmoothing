cmake_minimum_required(VERSION 3.23)
project(RawMouseAnalyzer)

set(CMAKE_CXX_STANDARD 14)

add_executable(RawMouseAnalyzer main.cpp)

# Specify the path to the directory containing JSONcpp's library
#find_library(JSONCPP_LIBRARY
#        NAMES jsoncpp
#        HINTS "C:/Users/flori/Desktop/International Minor/ParkinsonMouse/RawMouseAnalyzer/libraries/jsoncpp"
#        )

# Include directories for JSONcpp's header files
include_directories("libraries/rapidjson/include")

# Link your target against the JSONcpp library
#target_link_libraries(RawMouseAnalyzer ${JSONCPP_LIBRARY})