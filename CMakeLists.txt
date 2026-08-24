cmake_minimum_required(VERSION 3.15)
project(epfd_ras VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directory
include_directories(include)

# Core Library Sources (excluding GUI and Main)
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
    "src/simulation/*.cpp"
    "src/services/*.cpp"
    "src/utils/*.cpp"
)

add_library(epfd_core STATIC ${EPFD_CORE_SRCS})
target_include_directories(epfd_core PUBLIC include)

# Main Console Executable
add_executable(epfd_app src/main.cpp)
target_link_libraries(epfd_app PRIVATE epfd_core)

# C++/Qt Desktop GUI Target (Qt6 or Qt5)
find_package(Qt6 COMPONENTS Widgets Core Gui QUIET)
if (Qt6_FOUND)
    message(STATUS "Found Qt6 (${Qt6_VERSION}). Building epfd_gui target.")
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    file(GLOB_RECURSE GUI_SRCS "src/gui/*.cpp" "src/gui/*.hpp")
    add_executable(epfd_gui ${GUI_SRCS})
    set_target_properties(epfd_gui PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
    target_include_directories(epfd_gui PRIVATE include src/gui)
    target_link_libraries(epfd_gui PRIVATE epfd_core Qt6::Widgets Qt6::Core Qt6::Gui)
else()
    find_package(Qt5 COMPONENTS Widgets Core Gui QUIET)
    if (Qt5_FOUND)
        message(STATUS "Found Qt5 (${Qt5_VERSION}). Building epfd_gui target.")
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
        file(GLOB_RECURSE GUI_SRCS "src/gui/*.cpp" "src/gui/*.hpp")
        add_executable(epfd_gui ${GUI_SRCS})
        set_target_properties(epfd_gui PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
        target_include_directories(epfd_gui PRIVATE include src/gui)
        target_link_libraries(epfd_gui PRIVATE epfd_core Qt5::Widgets Qt5::Core Qt5::Gui)
    else()
        message(STATUS "Qt6/Qt5 not detected in default PATH. epfd_gui target skipped.")
        message(STATUS "--> To build Qt GUI: cmake -DCMAKE_PREFIX_PATH=<path_to_qt> ..")
    endif()
endif()

# Test Executable
enable_testing()
file(GLOB_RECURSE TEST_SRCS "tests/*.cpp")
add_executable(epfd_tests ${TEST_SRCS})
target_include_directories(epfd_tests PRIVATE include tests)
target_link_libraries(epfd_tests PRIVATE epfd_core)

add_test(NAME AllTests COMMAND epfd_tests)
