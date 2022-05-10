cmake_minimum_required(VERSION 3.23)
project(witcher_senses)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(vendor)

include_directories(vendor/glad/include)

add_executable(witcher_senses src/main.cpp)
target_link_libraries(witcher_senses glad glfw)
