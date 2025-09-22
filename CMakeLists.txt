cmake_minimum_required(VERSION 3.16)
project(yagit C ASM)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE YAGIT_SOURCES "src/*.c")

set(ZSTD_ROOT "${PROJECT_SOURCE_DIR}/src/extern/zstd")
set(SHA256_ROOT "${PROJECT_SOURCE_DIR}/src/extern/sha256")

file(GLOB_RECURSE ZSTD_SOURCES
    "${ZSTD_ROOT}/*.c"
)

file(GLOB_RECURSE ZSTD_ASM_SOURCES "${ZSTD_ROOT}/*.S")

add_executable(yagit ${YAGIT_SOURCES} ${ZSTD_SOURCES} ${ZSTD_ASM_SOURCES})

target_compile_definitions(yagit PRIVATE ZSTD_ASM_SUPPORTED=1)

target_include_directories(yagit PRIVATE
    ${PROJECT_SOURCE_DIR}/src/include
    ${SHA256_ROOT}/
    ${ZSTD_ROOT}/lib
    ${ZSTD_ROOT}/lib/common
    ${ZSTD_ROOT}/lib/compress
    ${ZSTD_ROOT}/lib/decompress
)

target_link_libraries(yagit ws2_32)

target_compile_options(yagit PRIVATE -fvisibility=default)
