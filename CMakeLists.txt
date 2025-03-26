cmake_minimum_required(VERSION 3.15)

project(jetpwmon
        VERSION 0.1
        LANGUAGES CXX C
        DESCRIPTION "A simple power monitor for Jetson"
        HOMEPAGE_URL "https://github.com/nerdneilsfield/jetson-power-monitor"
)

# prevent building on root
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Building on root directory is not allowed")
endif()

# 只支持 Linux 环境下编译
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This library only supports Linux")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(./include)

add_library(jetpwmon SHARED src/jetpwmon.c)
target_link_libraries(jetpwmon PRIVATE pthread)
install(TARGETS jetpwmon DESTINATION lib)
install(DIRECTORY include/jetpwmon DESTINATION include)
add_library(jetpwmon_static STATIC src/jetpwmon.c)
target_link_libraries(jetpwmon_static PRIVATE pthread)
install(TARGETS jetpwmon_static DESTINATION lib)

option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

option(BUILD_CLI "Build CLI" ON)
if(BUILD_CLI)
    add_executable(jetpwmon_cli src/jetpwmon_cli.c)
    target_link_libraries(jetpwmon_cli PRIVATE jetpwmon)
    install(TARGETS jetpwmon_cli DESTINATION bin)
endif()

option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(SCIKIT "Build with scikit-build" ON)
if(BUILD_PYTHON_BINDINGS)
    find_package(Python 3.7 COMPONENTS Interpreter Development.Module REQUIRED)
    if(SKBUILD)
        find_package(pybind11 CONFIG REQUIRED)
    else()
        add_subdirectory(3rdparty/pybind11)
    endif()
    message(STATUS "Building Python bindings")
    pybind11_add_module(jetpwmon_py MODULE
        bindings/python/jetpwmon_pybind.cpp
    )
    target_include_directories(jetpwmon_py PRIVATE ./include)
    target_link_libraries(jetpwmon_py PRIVATE jetpwmon_static)
    set_target_properties(jetpwmon_py PROPERTIES
        OUTPUT_NAME "jetpwmon"
        PREFIX ""
    )
    install(TARGETS jetpwmon_py
        LIBRARY DESTINATION jetpwmon
        RUNTIME DESTINATION jetpwmon
    )
endif()




