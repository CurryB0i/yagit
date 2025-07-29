cmake_minimum_required(VERSION 3.16)

project(yagit C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(yagit ${SOURCES})

target_include_directories(yagit PRIVATE ${PROJECT_SOURCE_DIR}/include)
