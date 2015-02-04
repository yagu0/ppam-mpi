cmake_minimum_required(VERSION 2.8.5)
project(ppam_mpi_tests)

include("${CMAKE_SOURCE_DIR}/../shared_CMakeLists.txt")

#ppam tests

include_directories("${CMAKE_SOURCE_DIR}/../src")

file(GLOB_RECURSE test_ppam_src "${CMAKE_SOURCE_DIR}/*.c")
add_executable(test_ppam.exe ${test_ppam_src})
find_library(PPAM_LIBRARY ppam "${CMAKE_BINARY_DIR}")
#TODO: Rblas ?!
target_link_libraries(test_ppam.exe m cds ${PPAM_LIBRARY})
