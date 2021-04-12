cmake_minimum_required (VERSION 2.6)
project (TinyRenderer)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -Wpedantic")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
)
if (NOT CLANG_TIDY_EXE)
    message(STATUS "clang-tidy not found")
else()
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    #set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=modernize-*,cppcoreguidelines-*,readability-*,misc-*,bugprone-*,performance-*")
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")
file(GLOB SOURCES "src/*.cpp")
add_executable(TinyRenderer ${SOURCES})

set_target_properties(TinyRenderer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if (CLANG_TIDY_EXE)
    set_target_properties(
        TinyRenderer PROPERTIES
        CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
endif()

