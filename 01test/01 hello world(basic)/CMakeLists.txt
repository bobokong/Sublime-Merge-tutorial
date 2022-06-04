# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (Hello_cmake)

# https://stackoverflow.com/questions/8443980/compiling-with-cmake-and-include-debug-information
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_C_FLAGS -g2)

# cmake -DCMAKE_BUILD_TYPE=Debug
# cmake ... -DCMAKE_C_FLAGS="-g2" -DCMAKE_CXX_FLAGS="-g2" ...

# Add an executable
add_executable(Hello.exe main.cpp)
