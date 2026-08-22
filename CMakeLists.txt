cmake_minimum_required(VERSION 3.15)
project(epfd_ras VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directory
include_directories(include)

# Core Library Sources
file(GLOB_RECURSE EPFD_CORE_SRCS 
    "src/common/*.cpp"
    "src/models/*.cpp"
    "src/dsa/*.cpp"
    "src/validation/*.cpp"
    "src/features/*.cpp"
    "src/fraud/*.cpp"
    "src/risk/*.cpp"
    "src/decision/*.cpp"
    "src/ml/*.cpp"
    "src/feedback/*.cpp"
    "src/database/*.cpp"
    "src/services/*.cpp"
    "src/utils/*.cpp"
)

add_library(epfd_core STATIC ${EPFD_CORE_SRCS})
target_include_directories(epfd_core PUBLIC include)

# Main Executable
add_executable(epfd_app src/main.cpp)
target_link_libraries(epfd_app PRIVATE epfd_core)

# Test Executable
enable_testing()
file(GLOB_RECURSE TEST_SRCS "tests/*.cpp")
add_executable(epfd_tests ${TEST_SRCS})
target_include_directories(epfd_tests PRIVATE include tests)
target_link_libraries(epfd_tests PRIVATE epfd_core)

add_test(NAME AllTests COMMAND epfd_tests)
