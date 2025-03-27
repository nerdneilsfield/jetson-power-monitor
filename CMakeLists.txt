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

# 设置安装路径
set(CMAKE_INSTALL_PREFIX "/usr")
set(CMAKE_INSTALL_LIBDIR "lib/${CMAKE_LIBRARY_ARCHITECTURE}")
set(CMAKE_INSTALL_INCLUDEDIR "include")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add option for debug output
option(SHOW_ALL "Show all debug output" OFF)
if(SHOW_ALL)
    add_definitions(-DSHOW_ALL_DEBUG)
endif()

include_directories(./include)

add_library(jetpwmon SHARED src/jetpwmon.c)
target_link_libraries(jetpwmon PRIVATE pthread)
add_library(jetpwmon_static STATIC src/jetpwmon.c)
target_link_libraries(jetpwmon_static PRIVATE pthread)

option(BUILD_CLI "Build CLI" ON)
if(BUILD_CLI)
    find_package(Curses REQUIRED)
    if(CURSES_FOUND)
        add_executable(jetpwmon_cli src/jetpwmon_cli.c)
        target_include_directories(jetpwmon_cli PRIVATE ${CURSES_INCLUDE_DIRS})
        target_link_libraries(jetpwmon_cli PRIVATE jetpwmon_static ${CURSES_LIBRARIES})
    else()
        message(WARNING "Curses not found, skipping CLI build, install libncurses-dev")
    endif()
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
endif()

option(BUILD_CPP_BINDINGS "Build C++ bindings" ON)
if(BUILD_CPP_BINDINGS)
    add_library(jetpwmon_cpp SHARED src/jetpwmon++.cpp)
    target_link_libraries(jetpwmon_cpp PRIVATE jetpwmon_static)
    add_library(jetpwmon_static_cpp STATIC src/jetpwmon++.cpp)
    target_link_libraries(jetpwmon_static_cpp PRIVATE jetpwmon_static)
endif()

option(BUILD_EXAMPLES "Build examples" OFF)
option(USE_OMP "Use OpenMP" OFF)
option(USE_EIGEN "Use Eigen" ON)
if(BUILD_EXAMPLES)
    # find omp 
    find_package(OpenMP REQUIRED)
    add_executable(example_c example/c_example.c)
    target_link_libraries(example_c PRIVATE jetpwmon_static OpenMP::OpenMP_CXX)

    if(USE_EIGEN)
        find_package(Eigen3 REQUIRED)
        add_executable(matrix_multiply example/matrix_multiply.cpp)
        target_link_libraries(matrix_multiply PRIVATE jetpwmon_static Eigen3::Eigen)
    endif()

    if(BUILD_CPP_BINDINGS)
        add_executable(example_cpp example/cpp_example.cpp)
        target_link_libraries(example_cpp PRIVATE jetpwmon_static_cpp OpenMP::OpenMP_CXX)

        if(USE_EIGEN)
            add_executable(matrix_multiply_cpp example/matrix_multiply_cpp.cpp)
            target_link_libraries(matrix_multiply_cpp PRIVATE jetpwmon_static_cpp Eigen3::Eigen)
        endif()
    endif()
endif()

option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()

    add_executable(c_api_test tests/test_c_api.cpp)
    target_link_libraries(c_api_test PRIVATE jetpwmon_static GTest::gtest_main pthread)
    add_test(NAME c_api_test COMMAND c_api_test)
    if(BUILD_CPP_BINDINGS)
        add_executable(cpp_api_test tests/test_cpp_api.cpp)
        target_link_libraries(cpp_api_test PRIVATE jetpwmon_static_cpp GTest::gtest_main pthread)
        add_test(NAME cpp_api_test COMMAND cpp_api_test)
    endif()

    include(GoogleTest)
endif()

# 创建导出目标
add_library(jetpwmon::jetpwmon ALIAS jetpwmon)
add_library(jetpwmon::static ALIAS jetpwmon_static)
add_library(jetpwmon::jetpwmon_cpp ALIAS jetpwmon_cpp)
add_library(jetpwmon::static_cpp ALIAS jetpwmon_static_cpp)

# 安装所有目标到导出集
install(TARGETS
    jetpwmon jetpwmon_static
    EXPORT jetpwmon-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(BUILD_CPP_BINDINGS)
    install(TARGETS
        jetpwmon_cpp jetpwmon_static_cpp
        EXPORT jetpwmon-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# 生成并安装 CMake targets 文件
install(EXPORT jetpwmon-targets
    FILE jetpwmon-targets.cmake
    NAMESPACE jetpwmon::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jetpwmon
)

# 生成并安装 pkg-config 文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/jetpwmon.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/jetpwmon.pc"
    @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/jetpwmon.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# 生成并安装 CMake 配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/jetpwmon-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/jetpwmon-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/jetpwmon-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jetpwmon
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/jetpwmon-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/jetpwmon-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jetpwmon
)

# 安装头文件
install(DIRECTORY include/jetpwmon
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 安装可执行文件
if(BUILD_CLI)
    install(TARGETS jetpwmon_cli
        RUNTIME DESTINATION bin
    )
endif()

if(BUILD_PYTHON_BINDINGS)
    install(TARGETS jetpwmon_py
        LIBRARY DESTINATION lib/python3/dist-packages/jetpwmon
    )
endif()

# 安装示例程序
if(BUILD_EXAMPLES)
    install(TARGETS example_c
        RUNTIME DESTINATION bin
    )
    if(USE_EIGEN)
        install(TARGETS matrix_multiply
            RUNTIME DESTINATION bin
        )
    endif()
    if(BUILD_CPP_BINDINGS)
        install(TARGETS example_cpp
            RUNTIME DESTINATION bin
        )
        if(USE_EIGEN)
            install(TARGETS matrix_multiply_cpp
                RUNTIME DESTINATION bin
            )
        endif()
    endif()
endif()

# CPack 配置部分
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "libjetpwmon")
set(CPACK_PACKAGE_VENDOR "Qi Deng")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A simple power monitor for Jetson (Development files)")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "dengqi935@gmail.com")

# 设置依赖关系
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.17)")
if(BUILD_PYTHON_BINDINGS)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, python3-dev")
endif()
if(BUILD_EXAMPLES AND USE_EIGEN)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libeigen3-dev")
endif()
if(USE_OMP)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libomp-dev")
endif()
if(BUILD_CLI)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libncurses-dev")
endif()

# 设置包的分类和优先级
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# 只保留 dev 组件
set(CPACK_DEB_COMPONENT_INSTALL ON)

# 包含 CPack 模块
include(CPack)


