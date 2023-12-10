cmake_minimum_required(VERSION 3.26)

project(ORTOA-TEE 
  VERSION 0.1
  LANGUAGES C CXX
)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_subdirectory(src)