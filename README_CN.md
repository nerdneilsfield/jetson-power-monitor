# jetson-power-monitor

<p align="center">
  <a href="https://pypistats.org/packages/jetpwmon"><img alt="PyPI - Downloads" src="https://img.shields.io/pypi/dw/jetpwmon.svg" /></a>
  <a href="https://badge.fury.io/py/jetpwmon"><img alt="PyPI version" src="https://badge.fury.io/py/jetpwmon.svg" /></a>
  <a href="https://www.python.org/"><img alt="PyPI - Python Version" src="https://img.shields.io/pypi/pyversions/jetpwmon.svg" /></a>
  <a href="https://pypi.org/project/jetpwmon/"><img alt="PyPI - Format" src="https://img.shields.io/pypi/format/jetpwmon.svg" /></a>
  <a href="/LICENSE"><img alt="GitHub" src="https://img.shields.io/github/license/nerdneilsfield/jetson-power-monitor" /></a>
  <a href="https://snyk.io/advisor/python/jetpwmon"><img alt="jetpwmon" src="https://snyk.io/advisor/python/jetpwmon/badge.svg" /></a>
  <a href="https://crates.io/crates/jetpwmon"><img src="https://img.shields.io/crates/v/jetpwmon.svg?colorB=319e8c" alt="Version info"></a><br>
  <a href="https://github.com/nerdneilsfield/jetson-power-monitor/actions?query=workflow%3A%22CI+%26+CD%22"><img alt="CI & CD" src="https://github.com/nerdneilsfield/jetson-power-monitor/workflows/CI%20&%20CD/badge.svg" /></a>
  <a href="https://github.com/nerdneilsfield/jetson-power-monitor/actions/workflows/github-code-scanning/codeql"><img alt="CodeQL" src="https://github.com/nerdneilsfield/jetson-power-monitor/actions/workflows/github-code-scanning/codeql/badge.svg?branch=master" /></a>
</p>

[English](https://github.com/nerdneilsfield/jetson-power-monitor/blob/master/README.md) | 中文


一个用于 NVIDIA Jetson 设备的全面电源监控库，支持多种编程语言。

## 特性

- 实时电源消耗监控
- 支持多种编程语言（C/C++、Rust、Python）
- 通过包管理器轻松安装
- 底层访问电源指标
- 跨平台支持 Jetson 设备

## 安装

### Python

```bash
pip install jetpwmon
```

### Rust

在 `Cargo.toml` 中添加：

```toml
[dependencies]
jetpwmon = "0.1.0"
```

### C/C++

从 [Releases](https://github.com/yourusername/jetson-power-monitor/releases) 页面下载预编译的 `.deb` 包：

```bash
sudo dpkg -i jetpwmon_0.1.0_amd64.deb
```

或者使用 CMake 在你的项目中查找和链接库：

```cmake
find_package(jetpwmon REQUIRED)
target_link_libraries(your_target PRIVATE jetpwmon::jetpwmon)  # 使用动态库
# 或者
target_link_libraries(your_target PRIVATE jetpwmon::static)    # 使用静态库

# 对于 C++ 绑定
target_link_libraries(your_target PRIVATE jetpwmon::jetpwmon_cpp)  # 使用动态库
# 或者
target_link_libraries(your_target PRIVATE jetpwmon::static_cpp)    # 使用静态库
```

## 使用方法

### Python

```python
from jetpwmon import PowerMonitor

monitor = PowerMonitor()
power = monitor.get_power_consumption()
print(f"当前功耗: {power}W")
```

### Rust

```rust
use jetpwmon::PowerMonitor;

fn main() {
    let monitor = PowerMonitor::new().unwrap();
    let power = monitor.get_power_consumption().unwrap();
    println!("当前功耗: {}W", power);
}
```

### C/C++

```c
#include <jetpwmon.h>

int main() {
    PowerMonitor* monitor = power_monitor_new();
    double power = power_monitor_get_power_consumption(monitor);
    printf("当前功耗: %.2fW\n", power);
    power_monitor_free(monitor);
    return 0;
}
```

### C++

```cpp
#include <jetpwmon++.hpp>

int main() {
    PowerMonitor monitor;
    double power = monitor.get_power_consumption();
    std::cout << "当前功耗: " << power << "W" << std::endl;
    return 0;
}
```

## API 文档

### Python

```python
class PowerMonitor:
    def __init__(self) -> None
    def get_power_consumption(self) -> float
    def get_voltage(self) -> float
    def get_current(self) -> float
```

### Rust

```rust
pub struct PowerMonitor {
    // ...
}

impl PowerMonitor {
    pub fn new() -> Result<Self, Error>
    pub fn get_power_consumption(&self) -> Result<f64, Error>
    pub fn get_voltage(&self) -> Result<f64, Error>
    pub fn get_current(&self) -> Result<f64, Error>
}
```

### C/C++

```c
PowerMonitor* power_monitor_new(void);
double power_monitor_get_power_consumption(PowerMonitor* monitor);
double power_monitor_get_voltage(PowerMonitor* monitor);
double power_monitor_get_current(PowerMonitor* monitor);
void power_monitor_free(PowerMonitor* monitor);
```

## 从源码构建

### 前置要求

- CMake 3.10 或更高版本
- 支持 C++17 的 C++ 编译器
- Python 3.7 或更高版本（用于 Python 绑定）
- Rust 工具链（用于 Rust 绑定）

### 构建步骤

```bash
git clone https://github.com/yourusername/jetson-power-monitor.git
cd jetson-power-monitor
mkdir build && cd build
cmake ..
make
sudo make install
```

## 贡献

我们欢迎贡献！请查看我们的 [CONTRIBUTING.md](CONTRIBUTING.md) 获取详细信息，包括：

- 项目架构和实现细节
- 开发环境设置和指南
- 代码风格和测试要求
- Pull Request 流程
- 常见开发任务
- 发布流程

## 许可证

本项目采用 `BSD 3-Clause License` 许可证 - 详见 [LICENSE](LICENSE) 文件。

## 致谢

- NVIDIA Jetson 团队提供的出色硬件
- 所有为本项目做出贡献的开发者
- [jetson_stats](https://github.com/rbonghi/jetson_stats)

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=nnerdneilsfield/jetson-power-monitor&type=Date)](https://star-history.com/#nerdneilsfield/jetson-power-monitor&Date)