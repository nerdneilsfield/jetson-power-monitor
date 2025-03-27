# jetson-power-monitor

<p align="center">
  <a href="https://github.com/nerdneilsfield/jetson-power-monitor"><img alt="GitHub Repo stars" src="https://img.shields.io/github/stars/nerdneilsfield/jetson-power-monitor?style=social" /></a>
  <a href="https://github.com/nerdneilsfield/jetson-power-monitor"><img alt="GitHub issues" src="https://img.shields.io/github/issues/nerdneilsfield/jetson-power-monitor.svg" /></a>
  <a href="https://github.com/nerdneilsfield/jetson-power-monitor"><img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/nerdneilsfield/jetson-power-monitor.svg" /></a>
  <a href="https://github.com/nerdneilsfield/jetson-power-monitor"><img alt="GitHub contributors" src="https://img.shields.io/github/contributors/nerdneilsfield/jetson-power-monitor.svg" /></a>
</p>

<p align="center">
        <a href=""><img alt="Build With C" src="https://img.shields.io/badge/Made%20with-C-1f425f.svg" /></a>
        <a href=""><img alt="Build With C++" src="https://img.shields.io/badge/Made%20with-C++-1f425f.svg" /></a>
        <a href=""><img alt="Build With Python" src="https://img.shields.io/badge/Made%20with-Python-1f425f.svg" /></a>
        <a href=""><img alt="Build With Rust" src="https://img.shields.io/badge/Made%20with-Rust-1f425f.svg" /></a>
<p align="center">

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
jetpwmon = "0.1.2"
```

### C/C++

从 [Releases](https://github.com/yourusername/jetson-power-monitor/releases) 页面下载预编译的 `.deb` 包：

```bash
sudo dpkg -i jetpwmon_0.1.2_amd64.deb
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

**快速开始：获取瞬时读数**

这个示例展示了如何直接从设备获取当前的总功耗、电压和电流读数。

```python
import jetpwmon

# 创建一个电源监控实例
monitor = jetpwmon.PowerMonitor()

# 获取最新的数据快照
try:
    data = monitor.get_latest_data()
    
    # 访问总体读数
    total = data['total']
    print(f"当前的总功耗：{total['power']:.2f} W")
    print(f"当前的总线电压：{total['voltage']:.2f} V")
    print(f"当前的总电流：{total['current']:.2f} A")
    
    # 访问各个传感器的读数
    print("\n各个传感器的读数：")
    for sensor in data['sensors']:
        print(f"传感器 {sensor['name']}：")
        print(f"  功率：{sensor['power']:.2f} W")
        print(f"  电压：{sensor['voltage']:.2f} V")
        print(f"  电流：{sensor['current']:.2f} A")

except Exception as e:
    print(f"读取功率指标时出错：{e}")
    print("请确保 INA3221 设备已连接并可访问（权限？）。")
```

<br/>

<details>
<summary><strong>高级用法：在任务期间监控功耗</strong></summary>

这个示例演示了如何在任务开始前启动后台功耗采样，任务结束后停止它，并获取监控期间的详细统计信息（最小值、最大值、平均功耗、总能量）。

```python
import jetpwmon
import time
import numpy as np # 使用 numpy 来进行一个样本的 CPU 密集型任务

def cpu_intensive_task():
    """模拟一个 CPU 密集型任务"""
    print("开始 CPU 密集型任务...")
    # 为了更快的示例运行，减小了矩阵的大小
    size = 2000
    matrix1 = np.random.rand(size, size)
    matrix2 = np.random.rand(size, size)
    # 执行矩阵乘法
    result = np.dot(matrix1, matrix2)
    print("CPU 密集型任务完成。")

def monitor_power_consumption(task_func):
    """在任务执行期间监控功耗"""
    # 创建一个电源监控实例
    monitor = jetpwmon.PowerMonitor()

    try:
        # 可选：设置采样频率（例如，1000Hz）
        # 更高的频率提供更精细的数据，但会增加开销。
        # 请查看库文档或设备限制以获取有效/最佳值。
        monitor.set_sampling_frequency(1000)

        # 在开始新的监控期间之前重置统计信息
        monitor.reset_statistics()

        # 开始后台采样
        print("开始功耗采样...")
        monitor.start_sampling()

        # --- 执行你想要监控的任务 ---
        task_func()
        # --- 任务完成 ---

        # 可选：等待一小段时间以确保最后的样本被采集，
        # 这取决于任务的持续时间和采样频率。
        time.sleep(0.1)

        # 停止后台采样
        monitor.stop_sampling()
        print("停止功耗采样。")

        # 获取收集的统计信息
        stats = monitor.get_statistics()

        # --- 打印收集的统计信息 ---
        print("\n--- 功耗统计信息 ---")

        # 打印总功耗统计信息
        if 'total' in stats and 'power' in stats['total']:
            total_stats = stats['total']['power']
            print("总功耗：")
            # 为了安全起见，使用 .get() 来获取值，以防一些统计信息没有被计算
            print(f"  最小值：{total_stats.get('min', float('nan')):.2f} W")
            print(f"  最大值：{total_stats.get('max', float('nan')):.2f} W")
            print(f"  平均值：{total_stats.get('avg', float('nan')):.2f} W")
            print("总功耗统计信息：")
            # 使用 .get() 来获取值，以防一些统计信息没有被计算
            print(f"  最小值：{total_stats.get('min', float('nan')):.2f} W")
            print(f"  最大值：{total_stats.get('max', float('nan')):.2f} W")
            print(f"  平均值：{total_stats.get('avg', float('nan')):.2f} W")
            print(f"  总能量：{total_stats.get('total', float('nan')):.2f} J")
            print(f"  样本计数：{total_stats.get('count', 0)}")
        else:
            print("总功耗统计信息不可用。")

        # 打印每个传感器/通道的功耗信息
        if 'sensors' in stats:
            print("\n每个传感器的功耗信息：")
            for sensor in stats['sensors']:
                 if 'power' in sensor:
                     sensor_stats = sensor['power']
                     print(f"\n  传感器：{sensor.get('name', '未知')}")
                     print(f"    最小值：{sensor_stats.get('min', float('nan')):.2f} W")
                     print(f"    最大值：{sensor_stats.get('max', float('nan')):.2f} W")
                     print(f"    平均值：{sensor_stats.get('avg', float('nan')):.2f} W")
                     print(f"    总能量：{sensor_stats.get('total', float('nan')):.2f} J")
                     print(f"    样本计数：{sensor_stats.get('count', 0)}")
        else:
             print("\n每个传感器的统计信息不可用。")

    except Exception as e:
        print(f"\n监控期间发生错误：{e}")
        print("请确保 INA3221 设备已连接并且可以访问。")

# --- 运行监控示例 ---
print("Jetson 电源监控示例程序")
print("===================================")
monitor_power_consumption(cpu_intensive_task)
```

</details>

<br/>

### Rust

首先，在你的 `Cargo.toml` 文件中添加 `jetpwmon` 作为依赖项。根据需要调整路径或版本。

```toml
[dependencies]
jetpwmon = "0.1.2"

# 示例也使用了这些crate：
ndarray = "0.15" # 用于矩阵示例
rand = "0.8"     # 用于矩阵示例
```

**快速开始：获取最新的传感器读数**

这个示例展示了如何初始化监控器，并获取当前功率、电压和电流的单个快照，包括总体和个体传感器。注意使用 `unsafe` 来访问通过raw指针返回的每个传感器数据。

```rust
use jetpwmon::{PowerMonitor, PowerData, SensorData, Error};
use std::slice;

fn main() -> Result<(), Error> {
    println!("Jetson 电源监控器 - Rust 快速开始");
    println!("======================================");

    // 初始化电源监控器。这连接到硬件。
    // '?' 运算符传播任何错误（如设备未找到）。
    let monitor = PowerMonitor::new()?;
    println!("电源监控器初始化成功。");

    // 获取最新的瞬时数据快照
    let data: PowerData = monitor.get_latest_data()?;

    // --- 访问总体聚合数据（安全） ---
    // 'total' 字段是 PowerData 中的常规结构体。
    println!("\n--- 总体读数 ---");
    println!("总功率: {:.2} W", data.total.power);
    println!("总线电压: {:.2} V", data.total.voltage); // 通常是VIN
    println!("总电流: {:.2} A", data.total.current);
    println!("状态: {}", String::from_utf8_lossy(&data.total.status).trim_matches('\0'));

    // --- 访问个体传感器数据（需要不安全） ---
    // 'data.sensors' 是从C来的raw指针 (*mut SensorData)。
    // 我们需要一个不安全的块来解引用它，并创建一个安全的切片。
    println!("\n--- 个体传感器读数 ---");
    if !data.sensors.is_null() && data.sensor_count > 0 {
        // 从raw指针和计数创建一个安全的切片
        // 安全性：假设C库保证了'data.sensors'指向了有效的内存，包含'data.sensor_count'个元素，并且这个内存至少在
        // 'data' 变量返回时保持有效。
        let sensors_slice: &[SensorData] = unsafe {
            slice::from_raw_parts(data.sensors, data.sensor_count as usize)
        };

        // 现在遍历安全的切片
        for sensor in sensors_slice {
            // 将固定大小的u8数组（C字符串）转换为Rust字符串
            // 使用from_utf8_lossy是更安全的，因为它处理潜在的无效UTF-8字节。
            let name = String::from_utf8_lossy(&sensor.name)
                           .trim_matches('\0') // 移除空填充/终止符
                           .to_string();
            let status = String::from_utf8_lossy(&sensor.status).trim_matches('\0').to_string();

            println!(
                "  传感器: {:<15} | 功率: {:>6.2} W | 电压: {:>5.2} V | 电流: {:>6.2} A | 在线: {} | 状态: {}",
                name, sensor.power, sensor.voltage, sensor.current, sensor.online, status
            );
        }
    } else {
        println!("  没有个体传感器数据可用，或者指针为空。");
    }

    // 不需要明确的清理。`PowerMonitor` 结构体实现了 `Drop` 特征，
    // 这自动调用C清理函数，当 `monitor` 超出作用域时。
    println!("\n监控器将自动清理。");
    Ok(())
}
```

<br/>

<details>
<summary><strong>高级：在任务期间监控电源</strong></summary>

这个示例演示了如何启动后台采样，运行一个CPU密集型任务（跨线程的矩阵乘法），停止采样，并检索详细的统计信息。它突出了使用Result进行错误处理以及访问每个传感器统计信息所需的不安全块。

这个示例的依赖项：

```toml
[dependencies]
jetpwmon = { version = "0.1.2" } # 根据需要进行调整
ndarray = "0.15"
rand = "0.8"
```

```rust
use jetpwmon::{PowerMonitor, PowerStats, SensorStats, Error};
use std::{thread, time::Duration, slice, error::Error as StdError};
use ndarray::Array2;
use rand::Rng;

// 示例任务参数（根据需要进行调整）
const MATRIX_SIZE: usize = 1000; // 矩阵的大小
const NUM_THREADS: usize = 4;    // 并发任务的数量
const NUM_ITERATIONS: usize = 5; // 每个线程的工作量

/// 使用ndarray进行矩阵乘法的CPU密集型任务
fn matrix_multiply_task(thread_id: usize) {
    // println!("Thread {} starting...", thread_id); // 可选的日志记录
    let mut rng = rand::thread_rng();
    // 创建填充有随机数据的大型矩阵
    let mut a: Array2<f64> = Array2::from_shape_fn((MATRIX_SIZE, MATRIX_SIZE), |_| rng.gen());
    let b: Array2<f64> = Array2::from_shape_fn((MATRIX_SIZE, MATRIX_SIZE), |_| rng.gen());

    // 执行重复的乘法
    for _ in 0..NUM_ITERATIONS {
        a = a.dot(&b); // 矩阵乘法
    }
    // println!("Thread {} finished.", thread_id); // 可选的日志记录
}

// 在主函数中使用Box<dyn StdError>进行灵活的错误处理
fn main() -> Result<(), Box<dyn StdError>> {
    println!("Jetson Power Monitor - Rust Monitoring Example");
    println!("==========================================");

    // 初始化电源监控
    let monitor = PowerMonitor::new()?; // 使用'?'传播错误
    println!("电源监控已初始化。");

    // 设置所需的采样频率（例如，1000 Hz）
    let frequency = 1000;
    monitor.set_sampling_frequency(frequency)?;
    println!("设置采样频率为{} Hz。", frequency);

    // 重置任何先前收集的统计信息
    monitor.reset_statistics()?;
    println!("重置统计信息。");

    // 在单独的线程中启动后台采样（由C库管理）
    monitor.start_sampling()?;
    println!("开始电源采样...");

    // 记录任务开始时间
    let task_start_time = std::time::Instant::now();

    // --- 在多个线程中运行CPU密集型任务 ---
    let mut handles = vec![];
    for i in 0..NUM_THREADS {
        let handle = thread::spawn(move || {
            matrix_multiply_task(i);
        });
        handles.push(handle);
    }
    // 等待所有线程完成
    for handle in handles {
        handle.join().expect("任务线程发生了错误！");
    }
    // --- 任务完成 ---

    let task_duration = task_start_time.elapsed();
    println!("\n任务执行完成，用时：{:.2?}", task_duration);

    // 允许最后的样本被后台线程收集一小会儿
    thread::sleep(Duration::from_millis(100)); // 如果需要，可以调整

    // 停止后台采样线程
    monitor.stop_sampling()?;
    println!("停止电源采样。");

    // 检索已收集的统计信息
    let stats: PowerStats = monitor.get_statistics()?;

    // --- 打印统计信息 ---
    println!("\n--- 电源消耗统计 ---");

    // 打印总体聚合统计
    println!("总体电源消耗：");
    println!("  最小功率：{:.2} W", stats.total.power.min);
    println!("  最大功率：{:.2} W", stats.total.power.max);
    println!("  平均功率：{:.2} W", stats.total.power.avg);
    println!("  总能量：{:.2} J", stats.total.power.total); // 能量 = 平均功率 * 时长
    println!("  样本数量：{}", stats.total.power.count);
    // 如果需要，你也可以访问stats.total.voltage和stats.total.current

    // 打印每个传感器的统计信息（需要不安全）
    println!("\n每个传感器的电源消耗：");
    if !stats.sensors.is_null() && stats.sensor_count > 0 {
        // 从原始指针和计数创建一个安全的切片
        // 安全性：假设C库保证了'stats'的生命周期内，'stats.sensors'指向了有效的内存。
        let sensor_stats_slice: &[SensorStats] = unsafe {
            slice::from_raw_parts(stats.sensors, stats.sensor_count as usize)
        };

        // 遍历安全的切片
        for sensor_stat in sensor_stats_slice {
            // 将固定大小的u8数组（C字符串）转换为Rust字符串
            let name = String::from_utf8_lossy(&sensor_stat.name)
                           .trim_matches('\0')
                           .to_string();

            println!("\n  传感器：{}", name);
            println!("    最小功率：{:.2} W", sensor_stat.power.min);
            println!("    最大功率：{:.2} W", sensor_stat.power.max);
            println!("    平均功率：{:.2} W", sensor_stat.power.avg);
            println!("    总能量：{:.2} J", sensor_stat.power.total);
            println!("    样本数量：{}", sensor_stat.power.count);
            // 如果需要，你也可以访问sensor_stat.voltage和sensor_stat.current
        }
    } else {
        println!("  没有可用的每个传感器的统计信息或者指针为空。");
    }

    println!("\n监控完成，资源将被清理。");
    // `monitor`在这里超出了作用域，Drop特性调用了pm_cleanup()
    Ok(())
}
```

</details>

### C/C++

**编译**

1. **包含头文件：** 在C源文件中添加以下行：

    ```c
    #include <jetpwmon/jetpwmon.h>
    // 或根据项目结构调整路径：
    // #include "path/to/include/jetpwmon/jetpwmon.h"
    ```

2. **链接库：** 编译时，需要链接`libjetpwmon`库。假设库和头文件安装在标准系统路径或通过`-L`和`-I`指定的路径：

    ```bash
    # 基本编译
    gcc your_program.c -o your_program -ljetpwmon

    # 如果库/头文件在自定义位置：
    # gcc your_program.c -o your_program -I/path/to/jetpwmon/include -L/path/to/jetpwmon/lib -ljetpwmon

    # 可以使用pkg-config来查找库和头文件路径：
    # gcc your_program.c -o your_program -ljetpwmon `pkg-config --libs --cflags jetpwmon`

    # 如果需要，也可以添加其他库（如pthread用于线程，m用于数学，omp用于OpenMP）
    # 示例：使用OpenMP（类似下面的高级示例）：
    # gcc your_program.c -o your_program -I/path/to/include -L/path/to/lib -ljetpwmon -fopenmp -lm
    ```

**快速入门：获取最新的传感器读数**

这个示例展示了基本的生命周期：初始化库，获取当前传感器读数的单个快照，打印它们，并执行必要的清理。

```c
#include <stdio.h>
#include <stdlib.h> // 用于EXIT_FAILURE
#include <jetpwmon/jetpwmon.h> // 根据需要调整路径

int main() {
    pm_handle_t handle = NULL;     // 库实例的不透明句柄
    pm_power_data_t current_data; // 用于存储结果的结构体
    pm_error_t err;                // 用于存储错误代码的变量

    // 1. 初始化库
    // pm_init分配资源并发现传感器。
    // 它将必要的句柄存储在'handle'中，以便后续调用。
    err = pm_init(&handle);
    if (err != PM_SUCCESS) {
        // 使用pm_error_string获取可读的错误信息
        fprintf(stderr, "错误：初始化jetpwmon失败：%s (代码：%d)\n", pm_error_string(err), err);
        return EXIT_FAILURE;
    }
    printf("库初始化成功。\n");

    // 2. 获取最新的数据快照
    // 传递结构体的地址；库将填充它。
    // 'current_data.sensors'指针将指向库内部的缓冲区。
    err = pm_get_latest_data(handle, &current_data);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "错误：获取最新数据失败：%s (代码：%d)\n", pm_error_string(err), err);
        pm_cleanup(handle); // 在错误退出前清理资源
        return EXIT_FAILURE;
    }

    // 3. 打印总体聚合读数
    printf("\n--- 总体读数 ---\n");
    printf("总功率  : %.2f W\n", current_data.total.power);
    printf("总线电压  : %.2f V\n", current_data.total.voltage);
    printf("总电流  : %.2f A\n", current_data.total.current);
    // 假设'status'是一个以null结尾的C字符串
    printf("状态       : %s\n", current_data.total.status);

    // 4. 打印个体传感器读数
    printf("\n--- 个体传感器读数 ---\n");
    // 检查传感器指针是否有效且计数是否为正
    // 'current_data.sensors'指针由库管理，并指向
    // 'current_data.sensor_count'个元素。它通常在
    // 下一次调用修改此数据的库函数或pm_cleanup之前有效。
    if (current_data.sensors != NULL && current_data.sensor_count > 0) {
        for (int i = 0; i < current_data.sensor_count; ++i) {
            // 使用指针数组索引访问数据
            pm_sensor_data_t* sensor = &current_data.sensors[i];
            printf("  传感器: %-15s | 功率: %6.2f W | 电压: %5.2f V | 电流: %6.2f A | 在线: %s | 状态: %s\n",
                   sensor->name,    // 假设以null结尾的字符串
                   sensor->power,
                   sensor->voltage,
                   sensor->current,
                   sensor->online ? "是" : "否",
                   sensor->status); // 假设以null结尾的字符串
        }
    } else {
        printf("  没有个体传感器数据可用。\n");
    }

    // 5. 清理库资源（必要）
    // 这将释放内存并停止任何正在运行的后台线程。
    err = pm_cleanup(handle);
    if (err != PM_SUCCESS) {
        // 记录错误，但程序应该仍然终止。
        fprintf(stderr, "错误：清理jetpwmon失败：%s (代码：%d)\n", pm_error_string(err), err);
        return EXIT_FAILURE; // 指示清理过程中发生了错误
    }
    printf("\n库资源清理成功。\n");

    return 0; // 成功
}
```

<br/>

<details>
<summary><strong>高级：监控特定任务期间的功率</strong></summary>

本示例展示了监控特定任务期间功率消耗的完整工作流程。它初始化库，配置并开始采样，执行一个CPU密集型任务（使用OpenMP进行并行化），停止采样，检索收集的统计数据，打印它们，并清理。

注意：编译此示例时应启用OpenMP支持（例如，`gcc -fopenmp` ...）

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // 用于usleep()
#include <time.h>   // 用于使用clock_gettime计时任务
#include <omp.h>    // 用于OpenMP并行任务示例
#include <jetpwmon/jetpwmon.h> // 根据需要调整路径

// 示例CPU密集型任务（使用OpenMP的矩阵乘法）
void cpu_intensive_task() {
    printf("开始CPU密集型任务...\n");
    const int size = 1500; // 示例中适度的大小
    double *matrix1 = NULL, *matrix2 = NULL, *result = NULL;

    // 为矩阵分配内存
    matrix1 = (double*)malloc(size * size * sizeof(double));
    matrix2 = (double*)malloc(size * size * sizeof(double));
    result = (double*)malloc(size * size * sizeof(double));
    if (!matrix1 || !matrix2 || !result) {
        fprintf(stderr, "任务错误：分配矩阵内存失败。\n");
        // 在返回之前释放任何可能已分配的内存
        free(matrix1);
        free(matrix2);
        free(result);
        return; // 在分配失败时退出任务函数
    }

    // 使用OpenMP并行for初始化矩阵随机数据
    #pragma omp parallel for
    for (int i = 0; i < size * size; i++) {
        // 注意：rand()不是线程安全的，但对于演示可能是可以接受的。
        // 对于生产，应使用线程安全的随机数生成器或每线程种子。
        matrix1[i] = (double)rand() / RAND_MAX;
        matrix2[i] = (double)rand() / RAND_MAX;
    }

    // 使用OpenMP并行for并collapse执行矩阵乘法
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            for (int k = 0; k < size; k++) {
                sum += matrix1[i * size + k] * matrix2[k * size + j];
            }
            result[i * size + j] = sum;
        }
    }

    // 释放分配的内存
    free(matrix1);
    free(matrix2);
    free(result);
    printf("CPU密集型任务完成。\n");
}

int main() {
    pm_handle_t handle = NULL;     // 库实例句柄
    pm_power_stats_t stats;        // 用于存储收集的统计数据的结构体
    pm_error_t err;                // 错误代码变量
    struct timespec start_time, end_time; // 用于计时任务执行

    printf("Jetson Power Monitor - C 监控示例\n");
    printf("=========================================\n");

    // 1. 初始化库
    err = pm_init(&handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "错误：初始化失败: %s\n", pm_error_string(err));
        return EXIT_FAILURE;
    }
    printf("库初始化成功。\n");

    // 2. 设置采样频率（例如，1000 Hz）
    int target_frequency = 1000;
    err = pm_set_sampling_frequency(handle, target_frequency);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "错误：设置采样频率失败: %s\n", pm_error_string(err));
        pm_cleanup(handle); // 清理前退出
        return EXIT_FAILURE;
    }
    printf("设置采样频率为 %d Hz。\n", target_frequency);

    // 3. 重置统计数据（在开始测量区间前推荐）
    err = pm_reset_statistics(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "错误：重置统计数据失败: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return EXIT_FAILURE;
    }
    printf("统计数据重置。\n");

    // 4. 开始后台采样
    // 这会启动由库管理的线程来收集数据。
    err = pm_start_sampling(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "错误：开始采样失败: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return EXIT_FAILURE;
    }
    printf("开始功率采样...\n");

    // --- 执行要监控的任务 ---
    clock_gettime(CLOCK_MONOTONIC, &start_time); // 任务前获取时间
    cpu_intensive_task();                        // 运行实际任务
    clock_gettime(CLOCK_MONOTONIC, &end_time);   // 任务后获取时间
    // --- 任务完成 ---

    // 计算任务持续时间
    double task_duration_sec = (end_time.tv_sec - start_time.tv_sec) +
                               (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("\n任务执行完成，耗时: %.3f 秒\n", task_duration_sec);

    // 可选：等待一段短时间，以确保采样线程捕获最终时刻。
    // 根据采样频率和任务性质调整持续时间。
    usleep(100 * 1000); // 100 毫秒

    // 5. 停止后台采样
    err = pm_stop_sampling(handle);
    if (err != PM_SUCCESS) {
        // 记录错误，但继续获取统计数据
        fprintf(stderr, "警告：停止采样不干净: %s\n", pm_error_string(err));
    } else {
        printf("停止功率采样。\n");
    }

    // 6. 获取收集的统计数据
    // 传入'stats'结构体的地址；库将填充它。
    // 'stats.sensors'将指向库管理的内部缓冲区。
    err = pm_get_statistics(handle, &stats);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "错误：获取统计数据失败: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return EXIT_FAILURE;
    }

    // --- 打印统计数据 ---
    printf("\n--- 功率消耗统计数据 ---\n");
    printf("总功率消耗:\n");
    printf("  最小功率   : %.2f W\n", stats.total.power.min);
    printf("  最大功率   : %.2f W\n", stats.total.power.max);
    printf("  平均功率   : %.2f W\n", stats.total.power.avg);
    printf("  总能量     : %.2f J (平均功率 * 持续时间)\n", stats.total.power.total);
    printf("  样本计数   : %lu\n", stats.total.power.count);
    // 通过stats.total.voltage.*等访问总体电压/电流统计数据

    printf("\n每个传感器的功率消耗:\n");
    if (stats.sensors != NULL && stats.sensor_count > 0) {
        for (int i = 0; i < stats.sensor_count; ++i) {
            pm_sensor_stats_t* sensor_stat = &stats.sensors[i];
            // 假设'name'是一个以null结尾的C字符串
            printf("\n  传感器: %s\n", sensor_stat->name);
            printf("    最小功率   : %.2f W\n", sensor_stat->power.min);
            printf("    最大功率   : %.2f W\n", sensor_stat->power.max);
            printf("    平均功率   : %.2f W\n", sensor_stat->power.avg);
            printf("    总能量     : %.2f J\n", sensor_stat->power.total);
            printf("    样本计数   : %lu\n", sensor_stat->power.count);
             // 通过sensor_stat->voltage.*等访问每个传感器的电压/电流统计数据
        }
    } else {
        printf("  没有每个传感器的统计数据。\n");
    }

    // 7. 清理库资源（必需）
    err = pm_cleanup(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "错误：清理失败: %s\n", pm_error_string(err));
        return EXIT_FAILURE; // 指示清理过程中发生了错误
    }
    printf("\n库资源清理成功。\n");

    return 0; // 成功
}
```

</details>

<br/>

### C++

**编译**

1. **包含头文件：** 在源代码中使用C++ wrapper头文件：

    ```cpp
    #include <jetpwmon/jetpwmon++.hpp> // 使用C++头文件
    #include <stdexcept> // 用于捕获异常
    #include <iostream>  // 用于打印
    ```

2. **链接库：** 编译C++代码（确保启用了C++14或更高的标准），并链接到基础的`libjetpwmon` C库：

    ```bash
    # 使用g++编译，确保支持C++14
    g++ your_program.cpp -o your_program -std=c++14 -ljetpwmon

    # 如果库/头文件在自定义位置：
    # g++ your_program.cpp -o your_program -std=c++14 -I/path/to/jetpwmon/include -L/path/to/jetpwmon/lib -ljetpwmon

    # 添加其他必要的标志（例如，-pthread用于std::thread，Eigen路径/库）
    # g++ your_program.cpp -o your_program -std=c++14 -I/path/to/eigen -I/path/to/include -L/path/to/lib -ljetpwmon -pthread
    ```

**C++ Wrapper的关键特性：**

- **RAII (资源获取即初始化)：** `jetpwmon::PowerMonitor`对象在创建时自动初始化库（`pm_init`），并在销毁时（超出作用域时）清理资源（`pm_cleanup`）。不需要手动清理调用。
- **异常安全：** C API错误被转换为`std::runtime_error`异常，允许使用`try...catch`块进行标准C++错误处理。

**快速入门：获取最新的传感器读数（C++ Wrapper）**

这个示例展示了使用C++ wrapper初始化监控器，获取当前读数的快照，打印它们，并让RAII处理清理。

```cpp
#include <jetpwmon/jetpwmon++.hpp> // C++ wrapper头文件
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept> // 用于std::runtime_error
#include <cstring>   // 用于strnlen

// 辅助函数：安全地将 C 字符数组（可能未以 null 结尾）转换为 std::string
std::string c_char_to_string(const char* c_str, size_t max_len) {
    // 找到字符串的实际长度或在 max_len 处停止
    size_t len = strnlen(c_str, max_len);
    return std::string(c_str, len);
}

int main() {
    try {
        // 1. 初始化：创建 PowerMonitor 对象
        // 构造函数处理 pm_init() 并在失败时抛出 std::runtime_error
        jetpwmon::PowerMonitor monitor;
        std::cout << "电源监控器初始化成功（RAII）。" << std::endl;

        // 2. 获取最新数据：返回一个 jetpwmon::PowerData 对象
        // 该对象保存数据快照
        jetpwmon::PowerData data = monitor.getLatestData(); // C API 错误时抛出异常

        // 3. 访问并打印总体读数
        // 使用 getTotal() 方法，它返回 C 结构体的常量引用
        const pm_sensor_data_t& total_data = data.getTotal();
        std::cout << "\n--- 总体读数 ---" << std::endl;
        std::cout << "总功率：" << total_data.power << " W" << std::endl;
        std::cout << "总线电压：" << total_data.voltage << " V" << std::endl;
        std::cout << "总电流：" << total_data.current << " A" << std::endl;
        // 安全地转换 C 字符数组状态字段
        std::cout << "状态：" << c_char_to_string(total_data.status, sizeof(total_data.status)) << std::endl;

        // 4. 访问并打印各个传感器的读数
        std::cout << "\n--- 各个传感器的读数 ---" << std::endl;
        const pm_sensor_data_t* sensors_ptr = data.getSensors(); // 获取原始 C 指针
        int sensor_count = data.getSensorCount();

        // 安全性注意：从 getSensors() 获取的指针指向由底层 C 库管理的内存，
        // 假定仅暂时有效。请及时访问。
        if (sensors_ptr != nullptr && sensor_count > 0) {
            for (int i = 0; i < sensor_count; ++i) {
                const pm_sensor_data_t& sensor = sensors_ptr[i]; // 通过指针访问
                std::cout << "  传感器: " << c_char_to_string(sensor.name, sizeof(sensor.name))
                          << " | 功率: " << sensor.power << " W"
                          << " | 电压: " << sensor.voltage << " V"
                          << " | 电流: " << sensor.current << " A"
                          << " | 在线: " << (sensor.online ? "是" : "否")
                          << " | 状态: " << c_char_to_string(sensor.status, sizeof(sensor.status))
                          << std::endl;
            }
        } else {
            std::cout << "  没有个体传感器数据可用。" << std::endl;
        }

        // 5. 清理是自动的！
        // 当'monitor'在'main'的末尾（或try块）超出作用域时，
        // 它的析构函数将被调用，这将自动调用pm_cleanup()。
        std::cout << "\n退出作用域。PowerMonitor析构函数将处理清理。" << std::endl;

    } catch (const std::runtime_error& e) {
        // 捕获PowerMonitor wrapper抛出的错误
        std::cerr << "错误: " << e.what() << std::endl;
        return 1; // 指示失败
    } catch (const std::exception& e) {
        // 捕获任何其他标准异常
        std::cerr << "发生了意外错误: " << e.what() << std::endl;
        return 1;
    }

    return 0; // 成功
}
```

<br/>

<details>
<summary><strong>高级：监控特定任务期间的功率（C++ Wrapper）</strong></summary>

本示例使用了`jetpwmon::PowerMonitor` C++ wrapper、`std::thread`和异常处理来监控特定任务期间的功率消耗（使用Eigen进行并行矩阵乘法）。

*示例依赖项：* Eigen库，C++11线程支持（`-pthread`）。

```cpp
#include <jetpwmon/jetpwmon++.hpp> // C++ wrapper
#include <iostream>
#include <vector>
#include <string>
#include <thread>         // 使用std::thread
#include <chrono>         // 用于计时和睡眠
#include <stdexcept>      // 用于异常处理
#include <Eigen/Dense>    // 用于Eigen矩阵任务
#include <cstring>        // 用于strnlen

// 示例任务参数
const int MATRIX_SIZE = 1000; // 根据系统内存/CPU调整
const int NUM_THREADS = 4;    // 任务的线程数
const int NUM_ITERATIONS = 5; // 每个线程的工作负载

// 安全地将C字符数组转换为std::string的辅助函数
std::string c_char_to_string(const char* c_str, size_t max_len) {
    size_t len = strnlen(c_str, max_len);
    return std::string(c_str, len);
}

// 使用Eigen库的示例CPU密集型任务
void eigen_matrix_task(int thread_id) {
    // std::cout << "线程 " << thread_id << " 正在开始Eigen任务..." << std::endl;
    Eigen::MatrixXd a = Eigen::MatrixXd::Random(MATRIX_SIZE, MATRIX_SIZE);
    Eigen::MatrixXd b = Eigen::MatrixXd::Random(MATRIX_SIZE, MATRIX_SIZE);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        a = a * b; // 执行矩阵乘法
    }
    // std::cout << "线程 " << thread_id << " 已完成Eigen任务。" << std::endl;
}

int main() {
    try {
        // 1. 初始化PowerMonitor（RAII处理pm_init）
        jetpwmon::PowerMonitor monitor;
        std::cout << "Power monitor已初始化。" << std::endl;

        // 2. 配置采样
        int frequency = 1000;
        monitor.setSamplingFrequency(frequency); // 抛出错误
        std::cout << "已将采样频率设置为 " << frequency << " Hz。" << std::endl;

        // 3. 重置统计数据
        monitor.resetStatistics(); // 抛出错误
        std::cout << "统计数据已重置。" << std::endl;

        // 4. 开始后台采样
        monitor.startSampling(); // 抛出错误
        std::cout << "已开始功率采样..." << std::endl;

        // --- 执行并行任务 ---
        auto task_start_time = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> task_threads;
        task_threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            // 使用std::thread进行C++并发
            task_threads.emplace_back(eigen_matrix_task, i);
        }
        // 等待所有任务线程完成
        for (auto& t : task_threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        auto task_end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> task_duration = task_end_time - task_start_time;
        std::cout << "\n任务执行完成，耗时: " << task_duration.count() << " 秒" << std::endl;
        // --- 任务完成 ---

        // 可选的最终样本收集暂停
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 5. 停止后台采样
        monitor.stopSampling(); // 抛出错误
        std::cout << "已停止功率采样。" << std::endl;

        // 6. 获取收集的统计数据
        // 返回一个jetpwmon::PowerStats对象。
        jetpwmon::PowerStats stats = monitor.getStatistics(); // 抛出错误

        // --- 打印统计数据 ---
        std::cout << "\n--- 功率消耗统计数据 ---" << std::endl;
        const pm_sensor_stats_t& total_stats = stats.getTotal();
        std::cout << "总功率消耗:" << std::endl;
        std::cout << "  最小功率   : " << total_stats.power.min << " W" << std::endl;
        std::cout << "  最大功率   : " << total_stats.power.max << " W" << std::endl;
        std::cout << "  平均功率   : " << total_stats.power.avg << " W" << std::endl;
        std::cout << "  总能量     : " << total_stats.power.total << " J" << std::endl;
        std::cout << "  样本数量   : " << total_stats.power.count << std::endl;

        std::cout << "\n每个传感器的功率消耗:" << std::endl;
        const pm_sensor_stats_t* sensors_stats_ptr = stats.getSensors();
        int sensor_count = stats.getSensorCount();

        // 安全注意：请及时访问指针，假设只在短时间内有效。
        if (sensors_stats_ptr != nullptr && sensor_count > 0) {
            for (int i = 0; i < sensor_count; ++i) {
                const pm_sensor_stats_t& sensor_stat = sensors_stats_ptr[i];
                std::cout << "\n  传感器: " << c_char_to_string(sensor_stat.name, sizeof(sensor_stat.name)) << std::endl;
                std::cout << "    最小功率   : " << sensor_stat.power.min << " W" << std::endl;
                std::cout << "    最大功率   : " << sensor_stat.power.max << " W" << std::endl;
                std::cout << "    平均功率   : " << sensor_stat.power.avg << " W" << std::endl;
                std::cout << "    总能量     : " << sensor_stat.power.total << " J" << std::endl;
                std::cout << "    样本数量   : " << sensor_stat.power.count << std::endl;
            }
        } else {
            std::cout << "  没有每个传感器的统计数据可用。" << std::endl;
        }

        // 7. 清理是自动的（RAII）
        std::cout << "\n监控完成。资源将被自动清理。" << std::endl;
        // 'monitor'析构函数将被自动调用，处理清理。

    } catch (const std::runtime_error& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1; // 指示失败
    } catch (const std::exception& e) {
        std::cerr << "发生了意外错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

</details>

<br/>

## API Documentation

### Python

<details>
<summary><strong>API参考</strong></summary>

以下是`PowerMonitor`类提供的主要方法：

```python
class PowerMonitor:
    def __init__(self) -> None:
        """
        初始化与电源监控硬件（例如，通过I2C的INA3221）的连接。
        如果设备找不到或无法访问，可能会引发异常。
        """
        pass # 实际实现被省略

    def get_power_consumption(self) -> float:
        """
        从设备读取当前总功率消耗，跨越相关通道。
        返回:
            float: 当前瞬时总功率，单位为瓦特。
        """
        pass

    def get_voltage(self) -> float:
        """
        从设备读取当前总线电压（通常来自特定通道，如VIN）。
        返回:
            float: 当前瞬时电压，单位为伏特。
        """
        pass

    def get_current(self) -> float:
        """
        从设备读取当前总电流，跨越相关通道。
        返回:
            float: 当前瞬时总电流，单位为安培。
        """
        pass

    def set_sampling_frequency(self, frequency_hz: int) -> None:
        """
        设置背景监控时的目标采样频率。
        参数:
            frequency_hz (int): 每秒的目标采样次数（例如，100，1000）。
                                实际可达到的速率可能会受到硬件/系统负载的限制。
        """
        pass

    def start_sampling(self) -> None:
        """
        启动一个背景线程或进程，以在配置的频率下连续采样电源数据。
        统计数据将内部累积。需要调用`stop_sampling()`以停止。
        """
        pass

    def stop_sampling(self) -> None:
        """
        停止由`start_sampling()`启动的背景采样过程。
        """
        pass

    def reset_statistics(self) -> None:
        """
        清除所有内部累积的统计数据（最小值、最大值、总和用于平均值、能量、计数）。
        在`start_sampling()`之前调用，以测量特定时间间隔。
        """
        pass

    def get_statistics(self) -> dict:
        """
        检索自上次重置或初始化以来累积的电源统计数据。
        最好在`start_sampling()`和`stop_sampling()`后使用。

        返回:
            dict: 包含聚合（'total'）和每个传感器（'sensors'）统计数据的字典。见下面文档的结构。返回空或部分填充的字典，如果采样没有运行或失败。
        """
        pass
```

</details>

<br/>

<details>
<summary><strong>统计数据结构（`get_statistics()`返回值）</strong></summary>

`get_statistics()`方法返回的字典结构如下：

```python
{
    'total': {  # 跨越相关电源供应传感器的统计数据聚合
        'power': {
            'min': float,   # 采样期间观察到的最小总功率（瓦特）
            'max': float,   # 采样期间观察到的最大总功率（瓦特）
            'avg': float,   # 采样期间的平均总功率（瓦特）
            'total': float, # 采样期间消耗的总能量（焦耳）
                            # 从平均功率和持续时间计算得出。
            'count': int    # 贡献到总统计数据的样本数量。
        }
        # 注意：可能还会包括'voltage'和'current'键
        # 如果这些也被聚合和跟踪。
    },
    'sensors': [  # 包含每个单独监控传感器/通道统计数据的列表
        {
            'name': str,    # 标识传感器的名称（例如，"VDD_GPU_SOC", "VDD_CPU_CV", "VIN_SYS_5V0")
                            # 名称取决于Jetson板模型和INA3221的配置。
            'power': {
                'min': float,   # 特定传感器的最小功率（瓦特）
                'max': float,   # 特定传感器的最大功率（瓦特）
                'avg': float,   # 特定传感器的平均功率（瓦特）
                'total': float, # 特定传感器的总能量（焦耳）
                'count': int    # 为该传感器收集的样本数量。
            },
            # 注意：可能还会包括'voltage'和'current'键
            # 如果这些被每个传感器监控。
        },
        # ... 可能还有更多传感器字典
    ]
}
```

**重要注意事项：**

- `'sensors'`列表中可用的确切传感器名称取决于特定的Jetson板模型和INA3221通道在库中的配置和命名方式。
- `'total'`能量通常是基于平均功率（`avg`）和采样周期的持续时间（从`count`和采样频率推导）计算的。
- 如果没有使用`start_sampling()`/`stop_sampling()`,或者数据收集失败，返回的字典可能为空、部分填充，或者包含默认值如`0`或`NaN`。健壮的代码应该能够处理可能缺失的键或非数字值（例如，使用`.get()`带默认值，如监控示例所示）。

</details>

### Rust

<details>
<summary><strong> Rust API 参考</strong></summary>

**结构体和枚举：**

- `PowerMonitor`: 库的主要接口。管理C句柄，并确保通过`Drop`特征进行清理。
- `SensorType`: 标识传感器类型的枚举（`Unknown`, `I2C`, `System`）。
- `SensorData`: 持有单个传感器的*瞬时*数据。
  - `name: [u8; 64]`: 传感器名称（C字符串，需要转换）。
  - `type_: SensorType`: 传感器类型。
  - `voltage: f64`, `current: f64`, `power: f64`: 测量值。
  - `online: bool`: 传感器当前是否可读。
  - `status: [u8; 32]`: 状态消息（C字符串，需要转换）。
  - `warning_threshold: f64`, `critical_threshold: f64`: 阈值（瓦特）。
- `Stats`: 持有单个度量的统计数据（最小值、最大值、平均值、总量、计数）。
- `SensorStats`: 持有一个传感器的统计数据，包含电压、电流和功率的`Stats`。
  - `name: [u8; 64]`: 传感器名称（C字符串，需要转换）。
  - `voltage: Stats`, `current: Stats`, `power: Stats`.
- `PowerData`: 持有瞬时数据快照。
  - `total: SensorData`: 跨越相关传感器的聚合数据。
  - `sensors: *mut SensorData`: **原始指针**指向`SensorData`数组。**需要`unsafe`块来访问**。
  - `sensor_count: i32`: `sensors`数组中的元素数量。
- `PowerStats`: 持有累积统计数据。
  - `total: SensorStats`: 跨越相关传感器的聚合统计数据。
  - `sensors: *mut SensorStats`: **原始指针**指向`SensorStats`数组。**需要`unsafe`块来访问**。
  - `sensor_count: i32`: `sensors`数组中的元素数量。
- `Error`: 枚举，表示基础C库可能的错误代码（例如，`InitFailed`, `NotRunning`, `NoSensors`）。实现了`From<i32>`和`Into<i32>`。

**`PowerMonitor` 方法：**

- `PowerMonitor::new() -> Result<Self, Error>`: 创建并初始化监控器实例。连接到硬件。
- `set_sampling_frequency(&self, frequency_hz: i32) -> Result<(), Error>`: 设置背景监控的目标采样频率（赫兹）。
- `get_sampling_frequency(&self) -> Result<i32, Error>`: 获取当前配置的采样频率。
- `start_sampling(&self) -> Result<(), Error>`: 启动背景采样线程。统计开始累积。
- `stop_sampling(&self) -> Result<(), Error>`: 停止背景采样线程。
- `is_sampling(&self) -> Result<bool, Error>`: 返回背景采样是否当前活跃。
- `get_latest_data(&self) -> Result<PowerData, Error>`: 获取最新的瞬时读数。**返回值（`PowerData`）包含原始指针，需要`unsafe`访问**。见"数据结构与安全注意事项"。
- `get_statistics(&self) -> Result<PowerStats, Error>`: 获取自上次`reset_statistics()`或初始化以来累积的统计数据。**返回值（`PowerStats`）包含原始指针，需要`unsafe`访问**。见"数据结构与安全注意事项"。
- `reset_statistics(&self) -> Result<(), Error>`: 重置所有内部统计计数器（最小值、最大值、平均值、总量、计数）为零。
- `get_sensor_count(&self) -> Result<i32, Error>`: 返回库检测到的传感器数量。
- `get_sensor_names(&self) -> Result<Vec<String>, Error>`: 返回包含所有检测到的传感器名称的`Vec<String>`。内部处理C字符串转换。

**错误处理：**

- 所有与C库交互的方法返回`Result<T, jetpwmon::Error>`.
- 使用标准Rust错误处理（例如，`match`, `if let Ok/Err`, `?`操作符）来检查和处理可能的错误，如设备访问失败、无效状态等。

**资源管理：**

- `PowerMonitor`结构体实现了`Drop`特征。`PowerMonitor`实例超出作用域时，它的`drop`方法会自动调用，进而调用C库的清理函数（`pm_cleanup`）。您不需要手动调用清理函数。

</details>

<br/>

<details>
<summary><strong>数据结构与安全注意事项</strong></summary>

**在`PowerData`和`PowerStats`中使用原始指针**

C库通过原始指针（`*mut SensorData`或`*mut SensorStats`）返回传感器数据/统计信息的数组。Rust包装器在`PowerData`和`PowerStats`结构体中直接公开这些原始指针。

**访问这些数据需要在您的代码中使用`unsafe`块。** 推荐的方式是从原始指针和计数创建一个临时的、安全的Rust切片：

```rust
use std::slice;
use jetpwmon::{PowerStats, SensorStats, Error}; // 假设这些已经定义

fn print_sensor_stats(stats: &PowerStats) -> Result<(), Error> {
    // 检查指针是否有效，计数是否为正
    if !stats.sensors.is_null() && stats.sensor_count > 0 {
        // 安全性：这个块假设C库保证：
        // 1. `stats.sensors`指向有效的内存。
        // 2. 内存中包含`stats.sensor_count`个初始化的`SensorStats`元素。
        // 3. 这段内存在`stats`引用的生命周期内保持有效。
        // 调用者必须维护这些不变量。
        let sensor_stats_slice: &[SensorStats] = unsafe {
            slice::from_raw_parts(stats.sensors, stats.sensor_count as usize)
        };

        // 现在'sensor_stats_slice'是一个安全的切片，您可以在上面进行迭代
        for sensor_stat in sensor_stats_slice {
            // 在这里安全地处理每个sensor_stat...
            // 记得处理C字符串'name'字段（见下文）
             let name = String::from_utf8_lossy(&sensor_stat.name).trim_matches('\0').to_string();
             println!("传感器: {}, 平均功率: {:.2} W", name, sensor_stat.power.avg);
        }
    } else {
        println!("没有可用的传感器统计信息。");
    }
    Ok(())
}

```

*在`PowerData`结构体中访问`sensors`时，同样的模式也适用。*

**在C字符串（`name`和`status`字段）中的工作**

结构体字段，如`name`（`[u8; 64]`）和`status`（`[u8; 32]`），是固定大小的字节数组，用于保存C风格的以空字符结尾的字符串（或者可能只是用空字符填充）。

要安全地将它们转换为Rust的`String`：

1. 使用`String::from_utf8_lossy()`：这个方法会优雅地处理潜在的无效UTF-8序列，将它们替换为`�`字符。
2. 使用`.trim_matches('\0')`：这个方法会移除C缓冲区中用于填充或终止的前导/尾随空字节。

```rust
use jetpwmon::SensorData; // 假设SensorData有一个name: [u8; 64]

fn get_name(sensor_data: &SensorData) -> String {
    String::from_utf8_lossy(&sensor_data.name) // 处理无效的UTF-8
        .trim_matches('\0')                  // 移除空填充/终止符
        .to_string()                         // 将Cow<str>转换为String
}
```

**总体安全性**

- 这个Rust包装器在可能的情况下（使用`Result`，`Drop`进行清理）尽可能地安全。
- 但是，**通过FFI直接与C库交互，特别是处理从C返回的原始指针（`get_latest_data`，`get_statistics`）时，涉及到`unsafe`操作**，这是不可避免的。
- **用户有责任**在`unsafe`块中工作时，了解底层C库提供的内存管理和生命周期保证。错误的假设可能导致未定义的行为（崩溃，内存损坏）。如果可能，始终查阅C库的文档。

</details>

### C/C++

<details>
<summary><strong>API参考</strong></summary>

**句柄类型:**

- `pm_handle_t`: 一个不透明的指针（`struct pm_handle_s*`），代表库的一个初始化实例。由`pm_init()`返回，并且大多数其他函数都需要它。必须将其传递给`pm_cleanup()`以释放资源。

**枚举:**

- `pm_error_t`: 整数错误代码。`PM_SUCCESS` (0) 表示成功。负值表示错误。见`pm_error_string()`以获取描述。
  - `PM_SUCCESS = 0`
  - `PM_ERROR_INIT_FAILED = -1`
  - `PM_ERROR_NOT_INITIALIZED = -2`
  - `PM_ERROR_ALREADY_RUNNING = -3`
  - `PM_ERROR_NOT_RUNNING = -4`
  - `PM_ERROR_INVALID_FREQUENCY = -5`
  - `PM_ERROR_NO_SENSORS = -6`
  - `PM_ERROR_FILE_ACCESS = -7`
  - `PM_ERROR_MEMORY = -8`
  - `PM_ERROR_THREAD = -9`
- `pm_sensor_type_t`: 标识电源传感器的类型。
  - `PM_SENSOR_TYPE_UNKNOWN = 0`
  - `PM_SENSOR_TYPE_I2C = 1` (例如，INA3221)
  - `PM_SENSOR_TYPE_SYSTEM = 2` (例如，sysfs电源供应类)

**数据结构:**

- `pm_sensor_data_t`: 持有单个传感器的*瞬时*数据。
  - `char name[64]`: 空终止的传感器名称。
  - `pm_sensor_type_t type`: 传感器类型。
  - `double voltage`, `current`, `power`: 测量值（V，A，W）。
  - `bool online`: 指示传感器当前是否可读。
  - `char status[32]`: 空终止的状态字符串（例如，"OK"）。
  - `double warning_threshold`, `critical_threshold`: 电源阈值（W）。
- `pm_stats_t`: 持有基本统计信息。
  - `double min`, `max`, `avg`: 最小、最大、平均值。
  - `double total`: 值的总和（可以用于计算能量：能量 = 平均功率 * 持续时间）。
  - `uint64_t count`: 收集的样本数量。
- `pm_sensor_stats_t`: 持有单个传感器的统计信息。
  - `char name[64]`: 空终止的传感器名称。
  - `pm_stats_t voltage`, `current`, `power`: 每个度量的统计信息。
- `pm_power_data_t`: 由`pm_get_latest_data`填充的结构体。
  - `pm_sensor_data_t total`: 聚合的瞬时数据。
  - `pm_sensor_data_t* sensors`: 指向单个传感器数据数组的指针。**库管理内存**。该指针直到下一个相关库调用或`pm_cleanup`为止有效。不要释放这个指针。
  - `int sensor_count`: `sensors`数组中有效元素的数量。
- `pm_power_stats_t`: 由`pm_get_statistics`填充的结构体。
  - `pm_sensor_stats_t total`: 聚合的统计信息。
  - `pm_sensor_stats_t* sensors`: 指向单个传感器统计信息数组的指针。**库管理内存**。该指针直到下一个相关库调用或`pm_cleanup`为止有效。不要释放这个指针。
  - `int sensor_count`: `sensors`数组中有效元素的数量。

**核心函数:**

- `pm_error_t pm_init(pm_handle_t* handle)`:
  - 初始化库，发现传感器，分配资源。
  - 将不透明的库实例句柄存储在提供的`handle`地址。
  - **必须首先调用**。返回`PM_SUCCESS`表示成功。
- `pm_error_t pm_cleanup(pm_handle_t handle)`:
  - 停止采样（如果活动），释放与`handle`关联的所有资源。
  - **必须调用**，以防止资源泄露。
- `const char* pm_error_string(pm_error_t error)`:
  - 返回一个常量、人类可读的字符串，描述给定的错误代码。不要修改或释放返回的字符串。

**采样控制与状态:**

- `pm_error_t pm_set_sampling_frequency(pm_handle_t handle, int frequency_hz)`:
  - 设置背景监控线程的目标采样频率（以Hz为单位）。必须大于0。
- `pm_error_t pm_get_sampling_frequency(pm_handle_t handle, int* frequency_hz)`:
  - 检索当前配置的采样频率，存储在`frequency_hz`地址。
- `pm_error_t pm_start_sampling(pm_handle_t handle)`:
  - 启动背景采样线程。统计开始累积。返回`PM_ERROR_ALREADY_RUNNING`如果已经启动。
- `pm_error_t pm_stop_sampling(pm_handle_t handle)`:
  - 停止背景采样线程。返回`PM_ERROR_NOT_RUNNING`如果不运行。
- `pm_error_t pm_is_sampling(pm_handle_t handle, bool* is_sampling)`:
  - 检查背景采样线程是否活动，将结果（`true`或`false`）存储在`is_sampling`地址。

**数据与统计检索:**

- `pm_error_t pm_get_latest_data(pm_handle_t handle, pm_power_data_t* data)`:
  - 填充用户提供的`data`结构体，以获取最新的瞬时传感器读数。
  - `data->sensors`指针将指向库内部缓冲区。
- `pm_error_t pm_get_statistics(pm_handle_t handle, pm_power_stats_t* stats)`:
  - 填充用户提供的`stats`结构体，以获取自上次重置以来累积的统计信息。
  - `stats->sensors`指针将指向库内部缓冲区。
- `pm_error_t pm_reset_statistics(pm_handle_t handle)`:
  - 重置所有累积的统计信息（最小、最大、平均、总和、计数）为零。

**传感器信息:**

- `pm_error_t pm_get_sensor_count(pm_handle_t handle, int* count)`:
  - 获取库检测到的传感器总数。
- `pm_error_t pm_get_sensor_names(pm_handle_t handle, char** names, int* count)`:
  - 填充一个**调用者分配的**C字符串数组（`char* names[]`）以传感器名称。
  - `names`: 指向一个`char*`数组的指针。调用者必须分配这个数组。每个`char*`在数组中必须也指向一个调用者分配的缓冲区（例如，`char name_buffer[64]`），足以容纳传感器名称。
  - `count`: `[inout]`参数。在输入时，指向分配的`names`数组的大小。在输出时，指向实际写入的名称数量。
  - **注意:** 这个函数需要调用者进行仔细的内存管理。通过`pm_get_latest_data`或`pm_get_statistics`（使用`sensors[i].name`字段）访问名称通常更简单，因为库管理这些字符串。

</details>

### C++ Bindings

<details>
<summary><strong>API 参考（C++ 封装）</strong></summary>

**命名空间：** `jetpwmon`

**主类：** `PowerMonitor`

- **描述：** 一个用于管理`jetpwmon` C 库的 RAII 封装类。它在其构造函数中自动处理初始化（`pm_init`）和在其析构函数中自动处理清理（`pm_cleanup`）。它将 C API 错误代码转换为 `std::runtime_error` 异常。
- **资源管理：** 不可复制，但可移动。使用带有自定义删除器的 `std::unique_ptr` 来管理 C 句柄（`pm_handle_t`）。
- **构造函数：** `PowerMonitor()`
  - 初始化库连接。
  - **抛出：** 如果 `pm_init` 失败，抛出 `std::runtime_error`。异常的 `what()` 消息包含 `pm_error_string` 的错误描述。
- **析构函数：** `~PowerMonitor()`
  - 自动在管理的 C 句柄上调用 `pm_cleanup`。
- **方法：**
  - `void setSamplingFrequency(int frequency_hz)`
    - 设置背景采样频率（Hz）。
    - **抛出：** 如果 C API 失败，抛出 `std::runtime_error`。
  - `int getSamplingFrequency() const`
    - 获取当前的采样频率（Hz）。
    - **抛出：** 如果 C API 失败，抛出 `std::runtime_error`。
  - `void startSampling()`
    - 启动背景采样线程。
    - **抛出：** 如果 C API 失败（例如，已经运行），抛出 `std::runtime_error`。
  - `void stopSampling()`
    - 停止背景采样线程。
    - **抛出：** 如果 C API 失败（例如，未运行），抛出 `std::runtime_error`。
  - `bool isSampling() const`
    - 检查当前是否正在进行采样。
    - **抛出：** 如果 C API 失败，抛出 `std::runtime_error`。
  - `PowerData getLatestData() const`
    - 获取最新的瞬时传感器读数。
    - **返回：** 一个包含快照的 `PowerData` 对象。
    - **抛出：** 如果 C API 失败，抛出 `std::runtime_error`。
    - **注意：** 请参阅 `PowerData` 描述和关于指针有效性的安全注意事项。
  - `PowerStats getStatistics() const`
    - 获取自上次重置以来累积的统计信息。
    - **返回：** 一个包含统计信息的 `PowerStats` 对象。
    - **抛出：** 如果 C API 失败，抛出 `std::runtime_error`。
    - **注意：** 请参阅 `PowerStats` 描述和关于指针有效性的安全注意事项。
  - `void resetStatistics()`
    - 重置所有内部累积的统计信息。
    - **抛出：** 如果 C API 失败，抛出 `std::runtime_error`。
  - `int getSensorCount() const`
    - 获取检测到的传感器数量。
    - **抛出：** 如果 C API 失败，抛出 `std::runtime_error`。
  - `std::vector<std::string> getSensorNames() const`
    - 获取所有检测到的传感器的名称。处理 C 内存管理和字符串转换。
    - **返回：** 一个包含传感器名称的 `std::vector<std::string>`。
    - **抛出：** 如果 C API 失败，抛出 `std::runtime_error`。

**数据封装类：**

- `PowerData` / `PowerStats`
  - **描述：** `PowerData` 和 `PowerStats` 是围绕 C 结构体 `pm_power_data_t` 和 `pm_power_stats_t` 的薄封装，主要由 `getLatestData` 和 `getStatistics` 返回。它们不可复制，但可移动。
  - **内存：** 它们持有 `total` C 结构体成员的 *副本* 和 `sensors` 的 *原始 C 指针*（连同 `sensor_count`）。**它们不管理 `sensors` 指针指向的内存。**该内存由底层 C 库拥有。
  - **获取器：**
    - `const pm_sensor_data_t& getTotal() const`（对于 `PowerData`）
    - `const pm_sensor_stats_t& getTotal() const`（对于 `PowerStats`）
      - 返回对复制的 `total` 数据/统计结构体的 const 引用。
    - `const pm_sensor_data_t* getSensors() const`（对于 `PowerData`）
    - `const pm_sensor_stats_t* getSensors() const`（对于 `PowerStats`）
      - 返回到每个传感器数据/统计的原始 C 指针的数组。**请参阅安全注意事项。**
    - `int getSensorCount() const`：返回 `getSensors()` 指向的元素数量。

**底层 C 结构体：**

- C++ 封装通过 C 结构体（`pm_sensor_data_t`、`pm_stats_t`、`pm_sensor_stats_t`）提供对数据的访问。有关这些结构体中字段的详细描述，请参阅 C API 文档。

</details>

<br/>

<details>
<summary><strong>C++封装安全性和指针注意事项</strong></summary>

- **RAII和异常：** `PowerMonitor` 类通过其析构函数（RAII）自动清理资源（`pm_cleanup`）并将C错误代码转换为C++异常（`std::runtime_error`），显著提高了安全性。与`PowerMonitor`方法交互时，始终使用`try...catch`块。
- **指针有效性（`getSensors()`）：** `getLatestData()`和`getStatistics()`返回的`PowerData`和`PowerStats`对象包含对数组（`sensors`）的原始C指针。**关键是，C++封装类（`PowerData`，`PowerStats`）不管理这些指针指向的内存的生命周期。**这些内存由C库管理。
  - **假设：** `getSensors()`指向的内存通常只是**临时**有效的，可能仅在`PowerMonitor`对象的下一个非const调用之前或`PowerMonitor`对象被销毁之前。
  - **指导原则：** 在`getLatestData()`或`getStatistics()`调用后**立即**通过`getSensors()`返回的指针访问数据，**在相同的作用域内**。**不要**将这个原始指针存储以供以后使用，因为它可能会变得无效（悬空指针）。
- **C字符串处理：** 数据结构包含C风格的固定大小字符数组（例如，`name[64]`，`status[32]`）。使用安全方法（例如，使用`strnlen`的示例中的`c_char_to_string`辅助函数）将其转换为`std::string`，以避免缓冲区过读，特别是如果在固定大小内不能保证空终止。

</details>

## 从源代码构建

### 前提条件

- CMake 3.10 或更高版本
- 支持 C++17 的 C++ 编译器
- Python 3.8 或更高版本（用于 Python 绑定）
- Rust 工具链（用于 Rust 绑定）

### 构建步骤

#### C 库和 C++ 绑定

```bash
git clone https://github.com/nerdneilsfield/jetson-power-monitor.git
cd jetson-power-monitor
mkdir build && cd build
cmake ..
make
sudo make install
```

#### Python 绑定

```bash
python3 -m pip install setuptools pybind11
python3 -m pip install -e .

# 或者你需要构建 wheel
python3 -m pip install build
python3 -m build --wheel
# 结果将在 dist/ 目录下
```

#### Rust 绑定

```bash
# 将 C 头文件和源代码复制到 Rust 供应商目录
make copy-rust

# 构建 Rust crate
cd bindings/rust
cargo build
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
