cmake_minimum_required(VERSION 3.23)
project(witcher_senses)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(vendor)

include_directories(vendor/glad/include)
include_directories(vendor/gl/include)

file(GLOB SOURCE "src/**/*.cpp" "src/*.cpp")

add_executable(witcher_senses ${SOURCE})
target_link_libraries(witcher_senses gl glfw)
