cmake_minimum_required(VERSION 3.31)

# set the project name and version
project(VulkanProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Vulkan REQUIRED SPIRV-Tools)
include_directories(${Vulkan_INCLUDE_DIR})

add_subdirectory(engine)
add_subdirectory(project0)
add_subdirectory(project1)
add_subdirectory(project2)
add_subdirectory(project3)
