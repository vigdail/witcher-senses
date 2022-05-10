cmake_minimum_required(VERSION 3.23)
project(witcher_senses)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(vendor)

include_directories(vendor/glad/include)
include_directories(vendor/gl/include)
include_directories(vendor/assimp/include)
include_directories(vendor/entt/single_include)
#include_directories(vendor)

file(GLOB SOURCE "src/**/*.cpp" "src/*.cpp")

add_executable(witcher_senses ${SOURCE} src/mesh.cpp src/mesh.h)
target_link_libraries(witcher_senses gl glfw assimp)
