cmake_minimum_required(VERSION 3.10)

set(prj_name "flasher")

# Set the project name
project(${prj_name})

file(GLOB main_src "${CMAKE_SOURCE_DIR}/*.c")
# Add the executable
add_executable(${prj_name} "${main_src}")