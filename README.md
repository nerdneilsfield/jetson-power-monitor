# jetson-power-monitor

<p align="center">
  <a href="https://pypistats.org/packages/jetpwmon"><img alt="PyPI - Downloads" src="https://img.shields.io/pypi/dw/jetpwmon.svg" /></a>
  <a href="https://badge.fury.io/py/jetpwmon"><img alt="PyPI version" src="https://badge.fury.io/py/jetpwmon.svg" /></a>
  <a href="https://www.python.org/"><img alt="PyPI - Python Version" src="https://img.shields.io/pypi/pyversions/jetpwmon.svg" /></a>
  <a href="https://pypi.org/project/jetpwmon/"><img alt="PyPI - Format" src="https://img.shields.io/pypi/format/jetpwmon.svg" /></a>
  <a href="/LICENSE"><img alt="GitHub" src="https://img.shields.io/github/license/nerdneilsfield/jetson-power-monitor" /></a>
  <a href="https://snyk.io/advisor/python/jetpwmon"><img alt="jetpwmon" src="https://snyk.io/advisor/python/jetpwmon/badge.svg" /></a>
  <a href="https://crates.io/crates/jetpwmon"><img src="https://img.shields.io/crates/v/jetpwmon.svg?colorB=319e8c" alt="Version info"></a><br>
  <a href="https://github.com/nerdneilsfield/jetson-power-monitor/actions?query=workflow%3A%22Build%20ARM%20wheels%22"><img alt="Python" src="https://github.com/nerdneilsfield/jetson-power-monitor/workflows/Build%20ARM%20wheels/badge.svg" /></a>
  <a href="https://github.com/nerdneilsfield/jetson-power-monitor/actions?query=workflow%3A%22Build%20Ubuntu%20Packages%22"><img alt="Deb" src="https://github.com/nerdneilsfield/jetson-power-monitor/workflows/Build%20Ubuntu%20Packages/badge.svg" /></a>
  <a href="https://github.com/nerdneilsfield/jetson-power-monitor/actions?query=workflow%3A%22Publish%20Rust%20Crate%22"><img alt="Rust" src="https://github.com/nerdneilsfield/jetson-power-monitor/workflows/Publish%20Rust%20Crate/badge.svg" /></a>
</p>

English | [中文](https://github.com/nerdneilsfield/jetson-power-monitor/blob/master/README_CN.md)

A comprehensive power monitoring library for NVIDIA Jetson devices, available in multiple programming languages.

## Features

- Real-time power consumption monitoring
- Support for multiple programming languages (C/C++, Rust, Python)
- Easy installation through package managers
- Low-level access to power metrics
- Cross-platform support for Jetson devices

## Installation

### Python

```bash
pip install jetpwmon
```

### Rust

Add to your `Cargo.toml`:

```toml
[dependencies]
jetpwmon = "0.1.0"
```

### C/C++

Download the pre-built `.deb` package from the [Releases](https://github.com/yourusername/jetson-power-monitor/releases) page:

```bash
sudo dpkg -i jetpwmon_0.1.0_amd64.deb
```

Or use CMake to find and link the library in your project:

```cmake
find_package(jetpwmon REQUIRED)
target_link_libraries(your_target PRIVATE jetpwmon::jetpwmon)  # Use shared library
# or
target_link_libraries(your_target PRIVATE jetpwmon::static)    # Use static library

# For C++ bindings
target_link_libraries(your_target PRIVATE jetpwmon::jetpwmon_cpp)  # Use shared library
# or
target_link_libraries(your_target PRIVATE jetpwmon::static_cpp)    # Use static library
```

## Usage

### Python

```python
from jetpwmon import PowerMonitor

monitor = PowerMonitor()
power = monitor.get_power_consumption()
print(f"Current power consumption: {power}W")
```

### Rust

```rust
use jetpwmon::PowerMonitor;

fn main() {
    let monitor = PowerMonitor::new().unwrap();
    let power = monitor.get_power_consumption().unwrap();
    println!("Current power consumption: {}W", power);
}
```

### C/C++

```c
#include <jetpwmon.h>

int main() {
    PowerMonitor* monitor = power_monitor_new();
    double power = power_monitor_get_power_consumption(monitor);
    printf("Current power consumption: %.2fW\n", power);
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
    std::cout << "Current power consumption: " << power << "W" << std::endl;
    return 0;
}
```

## API Documentation

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

## Building from Source

### Prerequisites

- CMake 3.10 or higher
- C++ compiler with C++17 support
- Python 3.7 or higher (for Python bindings)
- Rust toolchain (for Rust bindings)

### Build Steps

```bash
git clone https://github.com/yourusername/jetson-power-monitor.git
cd jetson-power-monitor
mkdir build && cd build
cmake ..
make
sudo make install
```

## Contributing

We welcome contributions! Please see our [CONTRIBUTING.md](CONTRIBUTING.md) for detailed information about:

- Project architecture and implementation details
- Development setup and guidelines
- Code style and testing requirements
- Pull request process
- Common development tasks
- Release process

## License

This project is licensed under the `BSD 3-Clause License` License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- NVIDIA Jetson team for their excellent hardware
- All contributors who have helped with this project
- [jetson_stats](https://github.com/rbonghi/jetson_stats)

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=nnerdneilsfield/jetson-power-monitor&type=Date)](https://star-history.com/#nerdneilsfield/jetson-power-monitor&Date)
