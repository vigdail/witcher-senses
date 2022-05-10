cmake_minimum_required(VERSION 3.21)
project(sdf_playground CXX C)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

add_subdirectory(glfw)
add_subdirectory(glm)

include_directories(.)
include_directories(glfw/include)
include_directories(glad/include)
include_directories(glm)

set(GLAD_SOURCES glad/src/gl.c)

add_library(glad ${GLAD_SOURCES})
