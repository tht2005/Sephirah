cmake_minimum_required(VERSION 3.10)
project(Sephirah VERSION 1.0.0)

set(SEPHIRAH_NAME "Sephirah")
set(SEPHIRAH_AUTHOR "Nguyen Dinh Dang Duong")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

configure_file(
    ${CMAKE_SOURCE_DIR}/sephirah.h.in
    ${CMAKE_BINARY_DIR}/include/sephirah.h
)
include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(src)
add_subdirectory(tests)
