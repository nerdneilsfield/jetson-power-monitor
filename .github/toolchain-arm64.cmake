# .github/toolchain-arm64.cmake (修正版)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# 指定交叉编译器 (建议使用绝对路径增加稳定性)
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)

# --- 修正查找路径 ---
# 移除或注释掉错误的 CMAKE_FIND_ROOT_PATH 设置
# set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu) # <--- 注释掉或删除此行

# 直接将 arm64 的 include 和 lib 目录添加到 CMake 的搜索路径
# 将这些路径添加到现有路径的前面
list(PREPEND CMAKE_INCLUDE_PATH "/usr/include/aarch64-linux-gnu")
list(PREPEND CMAKE_LIBRARY_PATH "/usr/lib/aarch64-linux-gnu")

# --- 保持查找模式设置 ---
# 这确保 CMake 只在目标路径中查找，避免使用宿主机的库/头文件
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)