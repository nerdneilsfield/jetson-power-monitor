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

option(BUILD_CPP_BINDINGS "Build C++ bindings" ON)
if(BUILD_CPP_BINDINGS)
    add_library(jetpwmon_cpp SHARED src/jetpwmon++.cpp)
    target_link_libraries(jetpwmon_cpp PRIVATE jetpwmon_static)
    add_library(jetpwmon_static_cpp STATIC src/jetpwmon++.cpp)
    target_link_libraries(jetpwmon_static_cpp PRIVATE jetpwmon_static)
    install(TARGETS jetpwmon_cpp DESTINATION lib)
    install(TARGETS jetpwmon_static_cpp DESTINATION lib)
endif()


option(BUILD_EXAMPLES "Build examples" OFF)
if(BUILD_EXAMPLES)
    # find omp 
    find_package(OpenMP REQUIRED)
    add_executable(example_c example/c_example.c)
    target_link_libraries(example_c PRIVATE jetpwmon_static OpenMP::OpenMP_CXX)
    install(TARGETS example_c DESTINATION bin)
    if(BUILD_CPP_BINDINGS)
        add_executable(example_cpp example/cpp_example.cpp)
        target_link_libraries(example_cpp PRIVATE jetpwmon_static_cpp OpenMP::OpenMP_CXX)
        install(TARGETS example_cpp DESTINATION bin)
    endif()
endif()

# CPack 配置部分 - 放在 CMakeLists.txt 末尾
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "libjetpwmon")
set(CPACK_PACKAGE_VENDOR "Qi Deng")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A simple power monitor for Jetson")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "dengqi935@gmail.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.17)")
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# 创建开发包配置
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C/C++ Development Files")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Applications")

# 设置组件
set(CPACK_COMPONENTS_ALL libraries headers applications)
set(CPACK_DEB_COMPONENT_INSTALL ON)

# 将安装目标关联到组件
set_target_properties(jetpwmon PROPERTIES COMPONENT libraries)
set_target_properties(jetpwmon_static PROPERTIES COMPONENT libraries)

if(BUILD_CPP_BINDINGS)
  set_target_properties(jetpwmon_cpp PROPERTIES COMPONENT libraries)
  set_target_properties(jetpwmon_static_cpp PROPERTIES COMPONENT libraries)
endif()

if(BUILD_CLI)
  set_target_properties(jetpwmon_cli PROPERTIES COMPONENT applications)
endif()

# 头文件归入开发包
install(DIRECTORY include/jetpwmon DESTINATION include
        COMPONENT headers)

# 配置开发包
set(CPACK_DEBIAN_HEADERS_PACKAGE_NAME "libjetpwmon-dev")
set(CPACK_DEBIAN_HEADERS_PACKAGE_DEPENDS "libjetpwmon (= ${PROJECT_VERSION})")

# 包含 CPack 模块
include(CPack)

# Add option for debug output
option(SHOW_ALL "Show all debug output" OFF)

if(SHOW_ALL)
    target_compile_definitions(jetpwmon PRIVATE SHOW_ALL_DEBUG)
endif()

