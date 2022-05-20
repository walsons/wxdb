cmake_minimum_required(VERSION 3.10)

# set compiler
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# set the project name
project(wxdb)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# debug or release, use gdb to debug
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") 

# find all cpp files in src directory recursively 
# and set it to a variable named src_files
file(GLOB_RECURSE src_files src/*.cc main.cc)

# add the executable
add_executable(wxdb ${src_files})
