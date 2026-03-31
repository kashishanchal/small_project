cmake_minimum_required(VERSION 3.20)
project(content_fingerprinting_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++20")

# Enable Qt MOC and UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Verbosity
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Find packages
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# ============ Comparator Tool ============
# Compile the C++ comparator (no dependencies, pure C++)
add_executable(compare compare.cpp)
set_target_properties(compare PROPERTIES
    VERSION 1.0
    OUTPUT_NAME "compare"
)

# ============ GUI Dashboard ============
# Qt-based GUI application with Levenshtein distance matching
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(dashboard gui_dashboard_framework.cpp)

target_link_libraries(dashboard
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(dashboard PROPERTIES
    VERSION 1.0
    OUTPUT_NAME "dashboard"
    WIN32_EXECUTABLE ON
)

# ============ Testing ============
enable_testing()

# If test files are added later, uncomment:
# add_executable(test_fingerprinter tests/test_fingerprinter.cpp)
# target_link_libraries(test_fingerprinter PRIVATE fingerprinter)
# add_test(NAME fingerprinter_tests COMMAND test_fingerprinter)

# ============ Installation ============
install(TARGETS compare dashboard
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Install documentation
install(FILES
    SRS.md
    SDS.md
    TEST_PLAN.md
    README.md
    DASHBOARD_README.md
    DESTINATION share/doc/dashboard
)

# ============ Build Info ============
message(STATUS "")
message(STATUS "=== Content Fingerprinting Dashboard - Build Configuration ===")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt Version: ${Qt6_VERSION}")
message(STATUS "")
message(STATUS "Targets:")
message(STATUS "  - compare: CLI tool for content fingerprinting")
message(STATUS "  - dashboard: GUI application with Qt")
message(STATUS "")
message(STATUS "Build commands:")
message(STATUS "  cmake -B build -DCMAKE_BUILD_TYPE=Release")
message(STATUS "  cmake --build build -j")
message(STATUS "  cmake --install build --prefix /usr/local")
message(STATUS "")

