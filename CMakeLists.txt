cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(jsbjson VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(jsbjson)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
target_sources(json PRIVATE ${SOURCES})

# Include directories
target_include_directories(jsbjson PRIVATE "src")