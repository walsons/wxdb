cmake_minimum_required(VERSION 3.10)

# set compiler
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# set the project name
project(wxdb_unit_test)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find all cpp files in src directory recursively 
# and set it to a variable named src_files
file(GLOB_RECURSE src_files ../src/*.cc ./*.cc)

# add the executable
add_executable(unit_test ${src_files})
