cmake_minimum_required(VERSION 3.16)

project(sysmon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

qt_add_executable(sysmon ${SOURCES})

target_include_directories(sysmon PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(sysmon PRIVATE Qt6::Widgets)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(sysmon PRIVATE -Wall -Wextra)
endif()

find_package(Threads REQUIRED)
target_link_libraries(sysmon PRIVATE Threads::Threads)