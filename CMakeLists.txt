cmake_minimum_required(VERSION 3.10)
project(content_fingerprinting_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_SSDEEP "Enable ssdeep fuzzy hash support if available" OFF)

find_package(OpenSSL REQUIRED)

add_library(fingerprinter STATIC
    src/Fingerprinter.cpp
)

target_include_directories(fingerprinter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(fingerprinter PUBLIC OpenSSL::Crypto)

add_executable(test_fingerprinter tests/test_fingerprinter.cpp)
target_link_libraries(test_fingerprinter PRIVATE fingerprinter)

enable_testing()
add_test(NAME fingerprinter_tests COMMAND test_fingerprinter)

# comparator tool: compare clipboard/input against protected files
add_executable(compare_tool tools/compare.cpp)

