cmake_minimum_required(VERSION 3.31)

# set the project name and version
project(VulkanProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(engine)

# add the executable
add_executable(VulkanProject main.cpp)

target_link_libraries(VulkanProject PUBLIC Engine)

if(MSVC)
    set_property(TARGET VulkanProject APPEND PROPERTY LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
endif()