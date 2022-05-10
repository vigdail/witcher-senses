cmake_minimum_required(VERSION 3.21)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(gl)

include_directories(.)
include_directories(glfw/include)
include_directories(glm)
include_directories(gl/include)
