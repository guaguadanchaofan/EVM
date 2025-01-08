cmake_minimum_required(VERSION 3.0)
project(EVM)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")
include_directories(${CMAKE_SOURCE_DIR} /include)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


add_subdirectory(src)
add_subdirectory(example)