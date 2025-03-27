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
jetpwmon = "0.1.1"
```

### C/C++

Download the pre-built `.deb` package from the [Releases](https://github.com/yourusername/jetson-power-monitor/releases) page:

```bash
sudo dpkg -i jetpwmon_0.1.1_amd64.deb
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

**Quick Start: Get Instantaneous Readings**

This shows how to get the current total power consumption, voltage, and current readings directly from the device.

```python
import jetpwmon

# Create a power monitor instance
monitor = jetpwmon.PowerMonitor()

# Get the latest data snapshot
try:
    data = monitor.get_latest_data()
    
    # Access total readings
    total = data['total']
    print(f"Current total power consumption: {total['power']:.2f} W")
    print(f"Current bus voltage: {total['voltage']:.2f} V")
    print(f"Current total current: {total['current']:.2f} A")
    
    # Access individual sensor readings
    print("\nIndividual Sensor Readings:")
    for sensor in data['sensors']:
        print(f"Sensor {sensor['name']}:")
        print(f"  Power: {sensor['power']:.2f} W")
        print(f"  Voltage: {sensor['voltage']:.2f} V")
        print(f"  Current: {sensor['current']:.2f} A")

except Exception as e:
    print(f"Error reading power metrics: {e}")
    print("Ensure the INA3221 device is connected and accessible (permissions?).")
```

<br/>

<details>
<summary><strong>Advanced: Monitor Power During a Task</strong></summary>

This example demonstrates how to start background power sampling before a task, stop it afterwards, and retrieve detailed statistics (min, max, average power, total energy) for the monitoring period.

```python
import jetpwmon
import time
import numpy as np # Using numpy for a sample CPU-intensive task

def cpu_intensive_task():
    """Simulate a CPU-intensive task"""
    print("Starting CPU-intensive task...")
    # Reduced size for a quicker example run
    size = 2000
    matrix1 = np.random.rand(size, size)
    matrix2 = np.random.rand(size, size)
    # Perform matrix multiplication
    result = np.dot(matrix1, matrix2)
    print("CPU-intensive task completed.")

def monitor_power_consumption(task_func):
    """Monitor power consumption during task execution"""
    # Create a power monitor instance
    monitor = jetpwmon.PowerMonitor()

    try:
        # Optional: Set the sampling frequency (e.g., 1000Hz)
        # Higher frequencies provide more granular data but increase overhead.
        # Check library documentation or device limits for valid/optimal values.
        monitor.set_sampling_frequency(1000)

        # Reset statistics before starting a new monitoring period
        monitor.reset_statistics()

        # Start background sampling
        print("Starting power sampling...")
        monitor.start_sampling()

        # --- Execute the task you want to monitor ---
        task_func()
        # --- Task finished ---

        # Optional: Wait briefly to ensure last samples are captured,
        # depends on task duration and sampling frequency.
        time.sleep(0.1)

        # Stop background sampling
        monitor.stop_sampling()
        print("Stopped power sampling.")

        # Get collected statistics
        stats = monitor.get_statistics()

        # --- Print the collected statistics ---
        print("\n--- Power Consumption Statistics ---")

        # Print total power consumption statistics
        if 'total' in stats and 'power' in stats['total']:
            total_stats = stats['total']['power']
            print("Total Power Consumption:")
            # Use .get() for safety in case some stats weren't computed
            print(f"  Minimum Value: {total_stats.get('min', float('nan')):.2f} W")
            print(f"  Maximum Value: {total_stats.get('max', float('nan')):.2f} W")
            print(f"  Average Value: {total_stats.get('avg', float('nan')):.2f} W")
            print(f"  Total Energy: {total_stats.get('total', float('nan')):.2f} J")
            print(f"  Sample Count: {total_stats.get('count', 0)}")
        else:
            print("Total power statistics not available.")

        # Print power consumption information for each sensor/channel
        if 'sensors' in stats:
            print("\nPower Consumption Per Sensor:")
            for sensor in stats['sensors']:
                 if 'power' in sensor:
                     sensor_stats = sensor['power']
                     print(f"\n  Sensor: {sensor.get('name', 'Unknown')}")
                     print(f"    Minimum Value: {sensor_stats.get('min', float('nan')):.2f} W")
                     print(f"    Maximum Value: {sensor_stats.get('max', float('nan')):.2f} W")
                     print(f"    Average Value: {sensor_stats.get('avg', float('nan')):.2f} W")
                     print(f"    Total Energy: {sensor_stats.get('total', float('nan')):.2f} J")
                     print(f"    Sample Count: {sensor_stats.get('count', 0)}")
        else:
             print("\nPer-sensor statistics not available.")

    except Exception as e:
        print(f"\nAn error occurred during monitoring: {e}")
        print("Ensure the INA3221 device is connected and accessible.")

# --- Run the monitoring example ---
print("Jetson Power Monitor Example Program")
print("===================================")
monitor_power_consumption(cpu_intensive_task)

```

</details>

<br/>

### Rust

First, add `jetpwmon` as a dependency in your `Cargo.toml`. Adjust the path or version as needed.

```toml
[dependencies]
jetpwmon = "0.1.1"

# The examples also use these crates:
ndarray = "0.15" # For matrix example
rand = "0.8"     # For matrix example
```

**Quick Start: Get Latest Sensor Readings**

This example shows how to initialize the monitor and get a single snapshot of the current power, voltage, and current, for both the total and individual sensors. Note the use of unsafe to access per-sensor data returned via raw pointers.

```rust
use jetpwmon::{PowerMonitor, PowerData, SensorData, Error};
use std::slice;

fn main() -> Result<(), Error> {
    println!("Jetson Power Monitor - Rust Quick Start");
    println!("======================================");

    // Initialize the power monitor. This connects to the hardware.
    // The '?' operator propagates any errors (like device not found).
    let monitor = PowerMonitor::new()?;
    println!("Power monitor initialized successfully.");

    // Get the latest instantaneous data snapshot
    let data: PowerData = monitor.get_latest_data()?;

    // --- Access Total Aggregated Data (Safely) ---
    // The 'total' field is a regular struct within PowerData.
    println!("\n--- Total Readings ---");
    println!("Total Power: {:.2} W", data.total.power);
    println!("Bus Voltage: {:.2} V", data.total.voltage); // Often VIN
    println!("Total Current: {:.2} A", data.total.current);
    println!("Status: {}", String::from_utf8_lossy(&data.total.status).trim_matches('\0'));

    // --- Access Individual Sensor Data (Requires Unsafe) ---
    // 'data.sensors' is a raw pointer (*mut SensorData) from C.
    // We need an unsafe block to dereference it and create a safe slice.
    println!("\n--- Individual Sensor Readings ---");
    if !data.sensors.is_null() && data.sensor_count > 0 {
        // Create a safe slice from the raw pointer and count
        // SAFETY: Assumes the C library guarantees that 'data.sensors' points to valid memory
        // containing 'data.sensor_count' elements, and that this memory remains valid
        // at least for the lifetime of the 'data' variable returned by get_latest_data().
        let sensors_slice: &[SensorData] = unsafe {
            slice::from_raw_parts(data.sensors, data.sensor_count as usize)
        };

        // Now iterate over the safe slice
        for sensor in sensors_slice {
            // Convert the fixed-size u8 array (C string) to a Rust String
            // Using from_utf8_lossy is safer as it handles potential invalid UTF-8 bytes.
            let name = String::from_utf8_lossy(&sensor.name)
                           .trim_matches('\0') // Remove null padding/terminator
                           .to_string();
            let status = String::from_utf8_lossy(&sensor.status).trim_matches('\0').to_string();

            println!(
                "  Sensor: {:<15} | Pwr: {:>6.2} W | V: {:>5.2} V | I: {:>6.2} A | Online: {} | Status: {}",
                name, sensor.power, sensor.voltage, sensor.current, sensor.online, status
            );
        }
    } else {
        println!("  No individual sensor data available or pointer was null.");
    }

    // No explicit cleanup needed. The `PowerMonitor` struct implements the `Drop` trait,
    // which automatically calls the C cleanup function when `monitor` goes out of scope.
    println!("\nMonitor will be cleaned up automatically.");
    Ok(())
}
```

<br/>

<details>
<summary><strong>Advanced: Monitor Power During a Task</strong></summary>

This example demonstrates starting background sampling, running a CPU-intensive task (matrix multiplication across threads), stopping sampling, and retrieving detailed statistics. It highlights error handling with Result and the necessary unsafe block for accessing per-sensor statistics.

Dependencies needed for this example:

```toml
[dependencies]
jetpwmon = { version = "0.1.1" } # Adjust as needed
ndarray = "0.15"
rand = "0.8"
```

```rust
use jetpwmon::{PowerMonitor, PowerStats, SensorStats, Error};
use std::{thread, time::Duration, slice, error::Error as StdError};
use ndarray::Array2;
use rand::Rng;

// Example task parameters (adjust as needed)
const MATRIX_SIZE: usize = 1000; // Size of matrices
const NUM_THREADS: usize = 4;    // Number of concurrent tasks
const NUM_ITERATIONS: usize = 5; // Workload per thread

/// Example CPU-intensive task using ndarray for matrix multiplication
fn matrix_multiply_task(thread_id: usize) {
    // println!("Thread {} starting...", thread_id); // Optional logging
    let mut rng = rand::thread_rng();
    // Create large matrices filled with random data
    let mut a: Array2<f64> = Array2::from_shape_fn((MATRIX_SIZE, MATRIX_SIZE), |_| rng.gen());
    let b: Array2<f64> = Array2::from_shape_fn((MATRIX_SIZE, MATRIX_SIZE), |_| rng.gen());

    // Perform repeated multiplications
    for _ in 0..NUM_ITERATIONS {
        a = a.dot(&b); // Matrix multiplication
    }
    // println!("Thread {} finished.", thread_id); // Optional logging
}

// Use Box<dyn StdError> for flexible error handling in main
fn main() -> Result<(), Box<dyn StdError>> {
    println!("Jetson Power Monitor - Rust Monitoring Example");
    println!("==========================================");

    // Initialize the power monitor
    let monitor = PowerMonitor::new()?; // Propagate errors using '?'
    println!("Power monitor initialized.");

    // Set the desired sampling frequency (e.g., 1000 Hz)
    let frequency = 1000;
    monitor.set_sampling_frequency(frequency)?;
    println!("Set sampling frequency to {} Hz.", frequency);

    // Reset any previously collected statistics
    monitor.reset_statistics()?;
    println!("Reset statistics.");

    // Start background sampling in a separate thread (managed by the C library)
    monitor.start_sampling()?;
    println!("Started power sampling...");

    // Record task start time
    let task_start_time = std::time::Instant::now();

    // --- Run the CPU-intensive task across multiple threads ---
    let mut handles = vec![];
    for i in 0..NUM_THREADS {
        let handle = thread::spawn(move || {
            matrix_multiply_task(i);
        });
        handles.push(handle);
    }
    // Wait for all threads to complete
    for handle in handles {
        handle.join().expect("Task thread panicked!");
    }
    // --- Task finished ---

    let task_duration = task_start_time.elapsed();
    println!("\nTask execution finished in: {:.2?}", task_duration);

    // Allow a brief moment for the last samples to be collected by the background thread
    thread::sleep(Duration::from_millis(100)); // Adjust if needed

    // Stop the background sampling thread
    monitor.stop_sampling()?;
    println!("Stopped power sampling.");

    // Retrieve the collected statistics
    let stats: PowerStats = monitor.get_statistics()?;

    // --- Print the Statistics ---
    println!("\n--- Power Consumption Statistics ---");

    // Print total aggregated statistics
    println!("Total Power Consumption:");
    println!("  Min Power: {:.2} W", stats.total.power.min);
    println!("  Max Power: {:.2} W", stats.total.power.max);
    println!("  Avg Power: {:.2} W", stats.total.power.avg);
    println!("  Total Energy: {:.2} J", stats.total.power.total); // Energy = Avg Power * Duration
    println!("  Sample Count: {}", stats.total.power.count);
    // You can also access stats.total.voltage and stats.total.current if needed

    // Print per-sensor statistics (requires unsafe)
    println!("\nPer-Sensor Power Consumption:");
    if !stats.sensors.is_null() && stats.sensor_count > 0 {
        // Create a safe slice from the raw pointer and count
        // SAFETY: Assumes C library guarantees pointer validity for the lifetime of 'stats'.
        let sensor_stats_slice: &[SensorStats] = unsafe {
            slice::from_raw_parts(stats.sensors, stats.sensor_count as usize)
        };

        // Iterate over the safe slice
        for sensor_stat in sensor_stats_slice {
            // Convert C char array name to Rust String
            let name = String::from_utf8_lossy(&sensor_stat.name)
                           .trim_matches('\0')
                           .to_string();

            println!("\n  Sensor: {}", name);
            println!("    Min Power: {:.2} W", sensor_stat.power.min);
            println!("    Max Power: {:.2} W", sensor_stat.power.max);
            println!("    Avg Power: {:.2} W", sensor_stat.power.avg);
            println!("    Total Energy: {:.2} J", sensor_stat.power.total);
            println!("    Sample Count: {}", sensor_stat.power.count);
            // You can also access sensor_stat.voltage and sensor_stat.current if needed
        }
    } else {
        println!("  No per-sensor statistics available or pointer was null.");
    }

    println!("\nMonitoring complete. Resources will be cleaned up.");
    // `monitor` goes out of scope here, Drop trait calls pm_cleanup()
    Ok(())
}
```

</details>

### C/C++

**Compilation**

1. **Include Header:** Add the following line to your C source files:

    ```c
    #include <jetpwmon/jetpwmon.h>
    // Or adjust the path based on your project structure:
    // #include "path/to/include/jetpwmon/jetpwmon.h"
    ```

2. **Link Library:** When compiling, you need to link against the `libjetpwmon` library. Assuming the library and header files are installed in standard system paths or paths specified via `-L` and `-I`:

    ```bash
    # Basic compilation
    gcc your_program.c -o your_program -ljetpwmon

    # If library/includes are in custom locations:
    # gcc your_program.c -o your_program -I/path/to/jetpwmon/include -L/path/to/jetpwmon/lib -ljetpwmon

    # You can use pkg-config to find the library and include paths:
    # gcc your_program.c -o your_program -ljetpwmon `pkg-config --libs --cflags jetpwmon`

    # Add other libraries if needed (like pthread for threading, m for math, omp for OpenMP)
    # Example with OpenMP (like the advanced example below):
    # gcc your_program.c -o your_program -I/path/to/include -L/path/to/lib -ljetpwmon -fopenmp -lm
    ```

**Quick Start: Get Latest Sensor Readings**

This example demonstrates the basic lifecycle: initialize the library, get a single snapshot of current sensor readings, print them, and perform the mandatory cleanup.

```c
#include <stdio.h>
#include <stdlib.h> // For EXIT_FAILURE
#include <jetpwmon/jetpwmon.h> // Adjust path if necessary

int main() {
    pm_handle_t handle = NULL;     // Opaque handle for the library instance
    pm_power_data_t current_data; // Struct to hold the results
    pm_error_t err;                // Variable to store error codes

    // 1. Initialize the library
    // pm_init allocates resources and discovers sensors.
    // It stores the handle needed for subsequent calls in 'handle'.
    err = pm_init(&handle);
    if (err != PM_SUCCESS) {
        // Use pm_error_string to get a readable error message
        fprintf(stderr, "ERROR: Failed to initialize jetpwmon: %s (code: %d)\n", pm_error_string(err), err);
        return EXIT_FAILURE;
    }
    printf("Library initialized successfully.\n");

    // 2. Get the latest data snapshot
    // Pass the address of the struct; the library fills it.
    // The 'current_data.sensors' pointer will point to an internal library buffer.
    err = pm_get_latest_data(handle, &current_data);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to get latest data: %s (code: %d)\n", pm_error_string(err), err);
        pm_cleanup(handle); // Clean up resources before exiting on error
        return EXIT_FAILURE;
    }

    // 3. Print Total Aggregated Readings
    printf("\n--- Total Readings ---\n");
    printf("Total Power  : %.2f W\n", current_data.total.power);
    printf("Bus Voltage  : %.2f V\n", current_data.total.voltage);
    printf("Total Current: %.2f A\n", current_data.total.current);
    // Assume 'status' is a null-terminated C string
    printf("Status       : %s\n", current_data.total.status);

    // 4. Print Individual Sensor Readings
    printf("\n--- Individual Sensor Readings ---\n");
    // Check if the sensors pointer is valid and count is positive
    // The 'current_data.sensors' pointer is managed by the library and points
    // to 'current_data.sensor_count' elements. It's typically valid until
    // the next call to a library function that modifies this data or pm_cleanup.
    if (current_data.sensors != NULL && current_data.sensor_count > 0) {
        for (int i = 0; i < current_data.sensor_count; ++i) {
            // Access data using array indexing on the pointer
            pm_sensor_data_t* sensor = &current_data.sensors[i];
            printf("  Sensor: %-15s | Pwr: %6.2f W | V: %5.2f V | I: %6.2f A | Online: %s | Status: %s\n",
                   sensor->name,    // Assumes null-terminated string
                   sensor->power,
                   sensor->voltage,
                   sensor->current,
                   sensor->online ? "Yes" : "No",
                   sensor->status); // Assumes null-terminated string
        }
    } else {
        printf("  No individual sensor data available.\n");
    }

    // 5. Clean up library resources (MANDATORY)
    // This releases memory and stops any running background threads.
    err = pm_cleanup(handle);
    if (err != PM_SUCCESS) {
        // Log the error, but the program should still terminate.
        fprintf(stderr, "ERROR: Failed to clean up jetpwmon cleanly: %s (code: %d)\n", pm_error_string(err), err);
        return EXIT_FAILURE; // Indicate an error occurred during cleanup
    }
    printf("\nLibrary resources cleaned up successfully.\n");

    return 0; // Success
}
```

<br/>

<details>
<summary><strong>Advanced: Monitor Power During a Task</strong></summary>

This example demonstrates the complete workflow for monitoring power consumption during a specific task. It initializes the library, configures and starts sampling, executes a CPU-intensive task (using OpenMP for parallelization), stops sampling, retrieves the collected statistics, prints them, and cleans up.

Note: Compile this example with OpenMP support enabled (e.g., `gcc -fopenmp` ...)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For usleep()
#include <time.h>   // For timing task using clock_gettime
#include <omp.h>    // For OpenMP parallel task example
#include <jetpwmon/jetpwmon.h> // Adjust path if necessary

// Example CPU-intensive task (Matrix Multiplication using OpenMP)
void cpu_intensive_task() {
    printf("Starting CPU-intensive task...\n");
    const int size = 1500; // Moderate size for example
    double *matrix1 = NULL, *matrix2 = NULL, *result = NULL;

    // Allocate memory for matrices
    matrix1 = (double*)malloc(size * size * sizeof(double));
    matrix2 = (double*)malloc(size * size * sizeof(double));
    result = (double*)malloc(size * size * sizeof(double));
    if (!matrix1 || !matrix2 || !result) {
        fprintf(stderr, "Task ERROR: Failed to allocate memory for matrices.\n");
        // Free any potentially allocated memory before returning
        free(matrix1);
        free(matrix2);
        free(result);
        return; // Exit the task function on allocation failure
    }

    // Initialize matrices with random data using OpenMP parallel for
    #pragma omp parallel for
    for (int i = 0; i < size * size; i++) {
        // Note: rand() is not thread-safe, but for a demo it might be acceptable.
        // For production, use thread-safe RNGs or seed per thread.
        matrix1[i] = (double)rand() / RAND_MAX;
        matrix2[i] = (double)rand() / RAND_MAX;
    }

    // Perform matrix multiplication using OpenMP parallel for with collapse
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

    // Free allocated memory
    free(matrix1);
    free(matrix2);
    free(result);
    printf("CPU-intensive task completed.\n");
}

int main() {
    pm_handle_t handle = NULL;     // Library instance handle
    pm_power_stats_t stats;        // Struct to hold collected statistics
    pm_error_t err;                // Error code variable
    struct timespec start_time, end_time; // For timing the task execution

    printf("Jetson Power Monitor - C Monitoring Example\n");
    printf("=========================================\n");

    // 1. Initialize the library
    err = pm_init(&handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "ERROR: Initialization failed: %s\n", pm_error_string(err));
        return EXIT_FAILURE;
    }
    printf("Library initialized.\n");

    // 2. Set Sampling Frequency (e.g., 1000 Hz)
    int target_frequency = 1000;
    err = pm_set_sampling_frequency(handle, target_frequency);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to set sampling frequency: %s\n", pm_error_string(err));
        pm_cleanup(handle); // Cleanup before exit
        return EXIT_FAILURE;
    }
    printf("Set sampling frequency to %d Hz.\n", target_frequency);

    // 3. Reset Statistics (Recommended before starting a measurement interval)
    err = pm_reset_statistics(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to reset statistics: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return EXIT_FAILURE;
    }
    printf("Statistics reset.\n");

    // 4. Start Background Sampling
    // This starts a thread managed by the library to collect data.
    err = pm_start_sampling(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to start sampling: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return EXIT_FAILURE;
    }
    printf("Started power sampling...\n");

    // --- Execute the task to be monitored ---
    clock_gettime(CLOCK_MONOTONIC, &start_time); // Get time before task
    cpu_intensive_task();                        // Run the actual task
    clock_gettime(CLOCK_MONOTONIC, &end_time);   // Get time after task
    // --- Task Finished ---

    // Calculate task duration
    double task_duration_sec = (end_time.tv_sec - start_time.tv_sec) +
                               (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("\nTask execution finished in: %.3f seconds\n", task_duration_sec);

    // Optional: Wait a short moment to ensure the sampling thread captures final moments.
    // Adjust duration based on sampling frequency and task nature.
    usleep(100 * 1000); // 100 milliseconds

    // 5. Stop Background Sampling
    err = pm_stop_sampling(handle);
    if (err != PM_SUCCESS) {
        // Log error, but proceed to get statistics if possible
        fprintf(stderr, "WARNING: Failed to stop sampling cleanly: %s\n", pm_error_string(err));
    } else {
        printf("Stopped power sampling.\n");
    }

    // 6. Get Collected Statistics
    // Pass address of 'stats' struct; library fills it.
    // 'stats.sensors' will point to an internal buffer managed by the library.
    err = pm_get_statistics(handle, &stats);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to get statistics: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return EXIT_FAILURE;
    }

    // --- Print Statistics ---
    printf("\n--- Power Consumption Statistics ---\n");
    printf("Total Power Consumption:\n");
    printf("  Min Power   : %.2f W\n", stats.total.power.min);
    printf("  Max Power   : %.2f W\n", stats.total.power.max);
    printf("  Avg Power   : %.2f W\n", stats.total.power.avg);
    printf("  Total Energy: %.2f J (Avg Power * Duration)\n", stats.total.power.total);
    printf("  Sample Count: %lu\n", stats.total.power.count);
    // Access total voltage/current stats via stats.total.voltage.* etc.

    printf("\nPer-Sensor Power Consumption:\n");
    if (stats.sensors != NULL && stats.sensor_count > 0) {
        for (int i = 0; i < stats.sensor_count; ++i) {
            pm_sensor_stats_t* sensor_stat = &stats.sensors[i];
            // Assume 'name' is a null-terminated C string
            printf("\n  Sensor: %s\n", sensor_stat->name);
            printf("    Min Power   : %.2f W\n", sensor_stat->power.min);
            printf("    Max Power   : %.2f W\n", sensor_stat->power.max);
            printf("    Avg Power   : %.2f W\n", sensor_stat->power.avg);
            printf("    Total Energy: %.2f J\n", sensor_stat->power.total);
            printf("    Sample Count: %lu\n", sensor_stat->power.count);
             // Access per-sensor voltage/current stats via sensor_stat->voltage.* etc.
        }
    } else {
        printf("  No per-sensor statistics available.\n");
    }

    // 7. Clean up library resources (MANDATORY)
    err = pm_cleanup(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "ERROR: Cleanup failed: %s\n", pm_error_string(err));
        return EXIT_FAILURE; // Indicate failure on cleanup error
    }
    printf("\nLibrary resources cleaned up successfully.\n");

    return 0; // Success
}
```

</details>

<br/>

### C++

**Compilation**

1. **Include Header:** Use the C++ wrapper header file in your source code:

    ```cpp
    #include <jetpwmon/jetpwmon++.hpp> // Use the C++ header
    #include <stdexcept> // For catching exceptions
    #include <iostream>  // For printing
    ```

2. **Link Library:** Compile your C++ code (ensuring C++14 or later standard is enabled) and link against the underlying `libjetpwmon` C library:

    ```bash
    # Compile using g++ with C++14 support
    g++ your_program.cpp -o your_program -std=c++14 -ljetpwmon

    # If library/includes are in custom locations:
    # g++ your_program.cpp -o your_program -std=c++14 -I/path/to/jetpwmon/include -L/path/to/jetpwmon/lib -ljetpwmon

    # Add other necessary flags (e.g., -pthread for std::thread, Eigen paths/libs)
    # g++ your_program.cpp -o your_program -std=c++14 -I/path/to/eigen -I/path/to/include -L/path/to/lib -ljetpwmon -pthread
    ```

**Key Features of the C++ Wrapper:**

- **RAII (Resource Acquisition Is Initialization):** The `jetpwmon::PowerMonitor` object automatically initializes the library (`pm_init`) on creation and cleans up resources (`pm_cleanup`) upon destruction (when it goes out of scope). No manual cleanup calls are needed.
- **Exception Safety:** C API errors are converted into `std::runtime_error` exceptions, allowing for standard C++ error handling using `try...catch` blocks.

**Quick Start: Get Latest Sensor Readings (C++ Wrapper)**

This example demonstrates initializing the monitor using the C++ wrapper, getting a snapshot of current readings, printing them, and letting RAII handle cleanup.

```cpp
#include <jetpwmon/jetpwmon++.hpp> // C++ wrapper header
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept> // For std::runtime_error
#include <cstring>   // For strnlen

// Helper function to safely convert C char array (potentially not null-terminated) to std::string
std::string c_char_to_string(const char* c_str, size_t max_len) {
    // Find the actual length of the string or stop at max_len
    size_t len = strnlen(c_str, max_len);
    return std::string(c_str, len);
}

int main() {
    try {
        // 1. Initialize: Create PowerMonitor object.
        // Constructor handles pm_init() and throws std::runtime_error on failure.
        jetpwmon::PowerMonitor monitor;
        std::cout << "Power monitor initialized successfully (RAII)." << std::endl;

        // 2. Get Latest Data: Returns a jetpwmon::PowerData object.
        // This object holds the data snapshot.
        jetpwmon::PowerData data = monitor.getLatestData(); // Throws on C API error

        // 3. Access and Print Total Readings
        // Use the getTotal() method which returns a const reference to the C struct.
        const pm_sensor_data_t& total_data = data.getTotal();
        std::cout << "\n--- Total Readings ---" << std::endl;
        std::cout << "Total Power  : " << total_data.power << " W" << std::endl;
        std::cout << "Bus Voltage  : " << total_data.voltage << " V" << std::endl;
        std::cout << "Total Current: " << total_data.current << " A" << std::endl;
        // Safely convert C char array status field
        std::cout << "Status       : " << c_char_to_string(total_data.status, sizeof(total_data.status)) << std::endl;

        // 4. Access and Print Individual Sensor Readings
        std::cout << "\n--- Individual Sensor Readings ---" << std::endl;
        const pm_sensor_data_t* sensors_ptr = data.getSensors(); // Get raw C pointer
        int sensor_count = data.getSensorCount();

        // SAFETY NOTE: The pointer from getSensors() points to memory managed
        // by the underlying C library, assumed valid only temporarily. Access promptly.
        if (sensors_ptr != nullptr && sensor_count > 0) {
            for (int i = 0; i < sensor_count; ++i) {
                const pm_sensor_data_t& sensor = sensors_ptr[i]; // Access via pointer
                std::cout << "  Sensor: " << c_char_to_string(sensor.name, sizeof(sensor.name))
                          << " | Pwr: " << sensor.power << " W"
                          << " | V: " << sensor.voltage << " V"
                          << " | I: " << sensor.current << " A"
                          << " | Online: " << (sensor.online ? "Yes" : "No")
                          << " | Status: " << c_char_to_string(sensor.status, sizeof(sensor.status))
                          << std::endl;
            }
        } else {
            std::cout << "  No individual sensor data available." << std::endl;
        }

        // 5. Cleanup is Automatic!
        // When 'monitor' goes out of scope at the end of 'main' (or the try block),
        // its destructor is called, which automatically calls pm_cleanup().
        std::cout << "\nExiting scope. PowerMonitor destructor will handle cleanup." << std::endl;

    } catch (const std::runtime_error& e) {
        // Catch errors thrown by the PowerMonitor wrapper
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1; // Indicate failure
    } catch (const std::exception& e) {
        // Catch any other standard exceptions
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0; // Success
}
```

<br/>

<details>
<summary><strong>Advanced: Monitor Power During a Task (C++ Wrapper)</strong></summary>

This example uses the `jetpwmon::PowerMonitor` C++ wrapper, `std::thread`, and exception handling to monitor power consumption during a parallel matrix multiplication task (using Eigen).

*Example Dependencies:* Eigen library, C++11 thread support (`-pthread`).

```cpp
#include <jetpwmon/jetpwmon++.hpp> // C++ wrapper
#include <iostream>
#include <vector>
#include <string>
#include <thread>         // Use std::thread
#include <chrono>         // For timing and sleep
#include <stdexcept>      // For exception handling
#include <Eigen/Dense>    // For Eigen matrix task
#include <cstring>        // For strnlen

// Example Task Parameters
const int MATRIX_SIZE = 1000; // Adjust based on system memory/CPU
const int NUM_THREADS = 4;    // Number of threads for the task
const int NUM_ITERATIONS = 5; // Workload per thread

// Helper function to safely convert C char array to std::string
std::string c_char_to_string(const char* c_str, size_t max_len) {
    size_t len = strnlen(c_str, max_len);
    return std::string(c_str, len);
}

// Example CPU-intensive task using Eigen library
void eigen_matrix_task(int thread_id) {
    // std::cout << "Thread " << thread_id << " starting Eigen task..." << std::endl;
    Eigen::MatrixXd a = Eigen::MatrixXd::Random(MATRIX_SIZE, MATRIX_SIZE);
    Eigen::MatrixXd b = Eigen::MatrixXd::Random(MATRIX_SIZE, MATRIX_SIZE);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        a = a * b; // Perform matrix multiplication
    }
    // std::cout << "Thread " << thread_id << " finished Eigen task." << std::endl;
}

int main() {
    try {
        // 1. Initialize PowerMonitor (RAII handles pm_init)
        jetpwmon::PowerMonitor monitor;
        std::cout << "Power monitor initialized." << std::endl;

        // 2. Configure Sampling
        int frequency = 1000;
        monitor.setSamplingFrequency(frequency); // Throws on error
        std::cout << "Set sampling frequency to " << frequency << " Hz." << std::endl;

        // 3. Reset Statistics
        monitor.resetStatistics(); // Throws on error
        std::cout << "Statistics reset." << std::endl;

        // 4. Start Background Sampling
        monitor.startSampling(); // Throws on error
        std::cout << "Started power sampling..." << std::endl;

        // --- Execute the Parallel Task ---
        auto task_start_time = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> task_threads;
        task_threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            // Use std::thread for C++ concurrency
            task_threads.emplace_back(eigen_matrix_task, i);
        }
        // Wait for all task threads to complete
        for (auto& t : task_threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        auto task_end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> task_duration = task_end_time - task_start_time;
        std::cout << "\nTask execution finished in: " << task_duration.count() << " seconds" << std::endl;
        // --- Task Finished ---

        // Optional pause for final sample collection
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 5. Stop Background Sampling
        monitor.stopSampling(); // Throws on error
        std::cout << "Stopped power sampling." << std::endl;

        // 6. Get Collected Statistics
        // Returns a jetpwmon::PowerStats object.
        jetpwmon::PowerStats stats = monitor.getStatistics(); // Throws on error

        // --- Print Statistics ---
        std::cout << "\n--- Power Consumption Statistics ---" << std::endl;
        const pm_sensor_stats_t& total_stats = stats.getTotal();
        std::cout << "Total Power Consumption:" << std::endl;
        std::cout << "  Min Power   : " << total_stats.power.min << " W" << std::endl;
        std::cout << "  Max Power   : " << total_stats.power.max << " W" << std::endl;
        std::cout << "  Avg Power   : " << total_stats.power.avg << " W" << std::endl;
        std::cout << "  Total Energy: " << total_stats.power.total << " J" << std::endl;
        std::cout << "  Sample Count: " << total_stats.power.count << std::endl;

        std::cout << "\nPer-Sensor Power Consumption:" << std::endl;
        const pm_sensor_stats_t* sensors_stats_ptr = stats.getSensors();
        int sensor_count = stats.getSensorCount();

        // SAFETY NOTE: Access pointer promptly, assumes temporary validity.
        if (sensors_stats_ptr != nullptr && sensor_count > 0) {
            for (int i = 0; i < sensor_count; ++i) {
                const pm_sensor_stats_t& sensor_stat = sensors_stats_ptr[i];
                std::cout << "\n  Sensor: " << c_char_to_string(sensor_stat.name, sizeof(sensor_stat.name)) << std::endl;
                std::cout << "    Min Power   : " << sensor_stat.power.min << " W" << std::endl;
                std::cout << "    Max Power   : " << sensor_stat.power.max << " W" << std::endl;
                std::cout << "    Avg Power   : " << sensor_stat.power.avg << " W" << std::endl;
                std::cout << "    Total Energy: " << sensor_stat.power.total << " J" << std::endl;
                std::cout << "    Sample Count: " << sensor_stat.power.count << std::endl;
            }
        } else {
            std::cout << "  No per-sensor statistics available." << std::endl;
        }

        // 7. Cleanup is Automatic (RAII)
        std::cout << "\nMonitoring complete. Resources automatically cleaned up." << std::endl;
        // 'monitor' destructor called automatically when main returns.

    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
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
<summary><strong>API Reference</strong></summary>

Here are the primary methods available on the `PowerMonitor` class:

```python
class PowerMonitor:
    def __init__(self) -> None:
        """
        Initializes the connection to the power monitor hardware (e.g., INA3221 via I2C).
        May raise an exception if the device cannot be found or accessed.
        """
        pass # Actual implementation omitted

    def get_power_consumption(self) -> float:
        """
        Reads the device for the current total power consumption across relevant channels.
        Returns:
            float: Instantaneous total power in Watts.
        """
        pass

    def get_voltage(self) -> float:
        """
        Reads the device for the current bus voltage (typically from a specific channel like VIN).
        Returns:
            float: Instantaneous voltage in Volts.
        """
        pass

    def get_current(self) -> float:
        """
        Reads the device for the current total shunt current across relevant channels.
        Returns:
            float: Instantaneous total current in Amperes.
        """
        pass

    def set_sampling_frequency(self, frequency_hz: int) -> None:
        """
        Sets the target frequency for background sampling when monitoring.
        Args:
            frequency_hz (int): Desired samples per second (e.g., 100, 1000).
                                The actual achievable rate may be limited by hardware/system load.
        """
        pass

    def start_sampling(self) -> None:
        """
        Starts a background thread or process to continuously sample power data
        at the configured frequency. Statistics are accumulated internally.
        Requires `stop_sampling()` to be called later.
        """
        pass

    def stop_sampling(self) -> None:
        """
        Stops the background sampling process started by `start_sampling()`.
        """
        pass

    def reset_statistics(self) -> None:
        """
        Clears all internally accumulated statistics (min, max, sum for average, energy, count).
        Call this before `start_sampling()` to measure a specific interval.
        """
        pass

    def get_statistics(self) -> dict:
        """
        Retrieves the power statistics collected since the last reset or initialization.
        Best used after `start_sampling()` and `stop_sampling()`.

        Returns:
            dict: A dictionary containing aggregated ('total') and per-sensor ('sensors')
                  statistics. See the structure documented below. Returns empty or
                  partially filled dict if sampling didn't run or failed.
        """
        pass
```

</details>

<br/>

<details>
<summary><strong>Statistics Data Structure (`get_statistics()` return value)</strong></summary>

The `get_statistics()` method returns a dictionary structured as follows:

```python
{
    'total': {  # Statistics aggregated across relevant power-supplying sensors
        'power': {
            'min': float,   # Minimum total power observed during sampling (Watts)
            'max': float,   # Maximum total power observed during sampling (Watts)
            'avg': float,   # Average total power over the sampling period (Watts)
            'total': float, # Total energy consumed during the period (Joules)
                            # Calculated from average power and duration.
            'count': int    # Number of samples contributing to the total statistics.
        }
        # Note: May potentially include 'voltage' and 'current' keys
        # if these are also aggregated and tracked.
    },
    'sensors': [  # A list containing statistics for each individual monitored sensor/channel
        {
            'name': str,    # Name identifying the sensor (e.g., "VDD_GPU_SOC", "VDD_CPU_CV", "VIN_SYS_5V0")
                            # Names depend on Jetson model and INA3221 configuration.
            'power': {
                'min': float,   # Minimum power for this specific sensor (Watts)
                'max': float,   # Maximum power for this specific sensor (Watts)
                'avg': float,   # Average power for this specific sensor (Watts)
                'total': float, # Total energy for this specific sensor (Joules)
                'count': int    # Number of samples collected for this sensor.
            },
            # Note: May potentially include 'voltage' and 'current' keys
            # if these are monitored per sensor.
        },
        # ... potentially more sensor dictionaries
    ]
}
```

**Important Notes:**

- The exact sensor names available in the `'sensors'` list depend on the specific Jetson board model and how the INA3221 channels are configured and named within the library.
- The `'total'` energy is typically calculated based on the average power (`avg`) and the duration of the sampling period (derived from `count` and the sampling frequency).
- If `start_sampling()`/`stop_sampling()` were not used, or if data collection failed, the returned dictionary might be empty, partially filled, or contain default values like `0` or `NaN`. Robust code should handle potentially missing keys or non-numeric values (e.g., using `.get()` with defaults as shown in the monitoring example).

</details>

### Rust

<details>
<summary><strong> Rust API Reference</strong></summary>

**Structs & Enums:**

- `PowerMonitor`: The main interface to the library. Manages the C handle and ensures cleanup via the `Drop` trait.
- `SensorType`: Enum identifying the type of sensor (`Unknown`, `I2C`, `System`).
- `SensorData`: Holds *instantaneous* data for one sensor.
  - `name: [u8; 64]`: Sensor name (C string, needs conversion).
  - `type_: SensorType`: Type of the sensor.
  - `voltage: f64`, `current: f64`, `power: f64`: Measured values.
  - `online: bool`: Whether the sensor is currently readable.
  - `status: [u8; 32]`: Status message (C string, needs conversion).
  - `warning_threshold: f64`, `critical_threshold: f64`: Thresholds in Watts.
- `Stats`: Holds statistics (min, max, avg, total, count) for a single metric (like power, voltage, or current).
- `SensorStats`: Holds statistics for one sensor, containing `Stats` for voltage, current, and power.
  - `name: [u8; 64]`: Sensor name (C string, needs conversion).
  - `voltage: Stats`, `current: Stats`, `power: Stats`.
- `PowerData`: Holds instantaneous data snapshot.
  - `total: SensorData`: Aggregated data across relevant sensors.
  - `sensors: *mut SensorData`: **Raw pointer** to an array of `SensorData`. **Requires `unsafe`** to access.
  - `sensor_count: i32`: Number of elements in the `sensors` array.
- `PowerStats`: Holds accumulated statistics.
  - `total: SensorStats`: Aggregated stats across relevant sensors.
  - `sensors: *mut SensorStats`: **Raw pointer** to an array of `SensorStats`. **Requires `unsafe`** to access.
  - `sensor_count: i32`: Number of elements in the `sensors` array.
- `Error`: Enum representing possible error codes from the underlying C library (e.g., `InitFailed`, `NotRunning`, `NoSensors`). Implements `From<i32>` and `Into<i32>`.

**`PowerMonitor` Methods:**

- `PowerMonitor::new() -> Result<Self, Error>`: Creates and initializes the monitor instance. Connects to hardware.
- `set_sampling_frequency(&self, frequency_hz: i32) -> Result<(), Error>`: Sets the target sampling frequency in Hz for background monitoring.
- `get_sampling_frequency(&self) -> Result<i32, Error>`: Gets the currently configured sampling frequency.
- `start_sampling(&self) -> Result<(), Error>`: Starts background sampling thread. Statistics begin accumulating.
- `stop_sampling(&self) -> Result<(), Error>`: Stops the background sampling thread.
- `is_sampling(&self) -> Result<bool, Error>`: Returns `true` if background sampling is currently active.
- `get_latest_data(&self) -> Result<PowerData, Error>`: Fetches the most recent instantaneous readings. **Return value (`PowerData`) contains raw pointers requiring `unsafe` access.** See "Data Structures & Safety Notes".
- `get_statistics(&self) -> Result<PowerStats, Error>`: Fetches the statistics accumulated since the last `reset_statistics()` or initialization. **Return value (`PowerStats`) contains raw pointers requiring `unsafe` access.** See "Data Structures & Safety Notes".
- `reset_statistics(&self) -> Result<(), Error>`: Resets all internal statistics counters (min, max, avg, total, count) to zero.
- `get_sensor_count(&self) -> Result<i32, Error>`: Returns the number of sensors detected by the library.
- `get_sensor_names(&self) -> Result<Vec<String>, Error>`: Returns a `Vec<String>` containing the names of all detected sensors. Handles C string conversion internally.

**Error Handling:**

- All methods that interact with the C library return `Result<T, jetpwmon::Error>`.
- Use standard Rust error handling (e.g., `match`, `if let Ok/Err`, `?` operator) to check for and handle potential errors like device access failures, invalid states, etc.

**Resource Management:**

- The `PowerMonitor` struct implements the `Drop` trait. When a `PowerMonitor` instance goes out of scope, its `drop` method is automatically called, which in turn calls the C library's cleanup function (`pm_cleanup`). You do **not** need to call a cleanup function manually.

</details>

<br/>

<details>
<summary><strong>Data Structures & Safety Notes</strong></summary>

**Working with Raw Pointers in `PowerData` and `PowerStats`**

The C library returns arrays of sensor data/statistics via raw pointers (`*mut SensorData` or `*mut SensorStats`). The Rust wrapper exposes these directly within the `PowerData` and `PowerStats` structs.

**Accessing this data requires `unsafe` blocks in your code.** The recommended way is to create a temporary, safe Rust slice from the raw pointer and count:

```rust
use std::slice;
use jetpwmon::{PowerStats, SensorStats, Error}; // Assuming these are defined

fn print_sensor_stats(stats: &PowerStats) -> Result<(), Error> {
    // Check if the pointer is valid and count is positive
    if !stats.sensors.is_null() && stats.sensor_count > 0 {
        // SAFETY: This block assumes the C library guarantees that:
        // 1. `stats.sensors` points to valid memory.
        // 2. The memory contains exactly `stats.sensor_count` initialized `SensorStats` elements.
        // 3. This memory remains valid for the lifetime of the `stats` reference.
        // The caller must uphold these invariants.
        let sensor_stats_slice: &[SensorStats] = unsafe {
            slice::from_raw_parts(stats.sensors, stats.sensor_count as usize)
        };

        // Now 'sensor_stats_slice' is a safe slice you can iterate over
        for sensor_stat in sensor_stats_slice {
            // Process each sensor_stat safely here...
            // Remember to handle the C string 'name' field (see below)
             let name = String::from_utf8_lossy(&sensor_stat.name).trim_matches('\0').to_string();
             println!("Sensor: {}, Avg Power: {:.2} W", name, sensor_stat.power.avg);
        }
    } else {
        println!("No per-sensor statistics available.");
    }
    Ok(())
}

```

*The same pattern applies when accessing `sensors` within a `PowerData` struct.*

**Working with C Strings (`name` and `status` fields)**

Struct fields like `name` (`[u8; 64]`) and `status` (`[u8; 32]`) are fixed-size byte arrays intended to hold C-style null-terminated strings (or potentially just padded with nulls).

To safely convert them to a Rust `String`:

1. Use `String::from_utf8_lossy()`: This handles potential invalid UTF-8 sequences gracefully by replacing them with the  character.
2. Use `.trim_matches('\0')`: This removes any leading/trailing null bytes used for padding or termination in the C buffer.

```rust
use jetpwmon::SensorData; // Assuming SensorData has a name: [u8; 64]

fn get_name(sensor_data: &SensorData) -> String {
    String::from_utf8_lossy(&sensor_data.name) // Handles invalid UTF-8
        .trim_matches('\0')                  // Removes null padding/terminator
        .to_string()                         // Converts Cow<str> to String
}
```

**Overall Safety**

- This Rust wrapper aims to be safe where possible (using `Result`, `Drop` for cleanup).
- However, **direct interaction with the C library via FFI inherently involves `unsafe` operations**, especially when dealing with raw pointers returned from C (`get_latest_data`, `get_statistics`).
- **It is the user's responsibility** to understand the memory management and lifetime guarantees provided by the underlying C library when working within `unsafe` blocks. Incorrect assumptions can lead to undefined behavior (crashes, memory corruption). Always consult the C library's documentation if available.

</details>

### C/C++

<details>
<summary><strong>API Reference</strong></summary>

**Handle Type:**

- `pm_handle_t`: An opaque pointer (`struct pm_handle_s*`) representing an initialized instance of the library. Returned by `pm_init()` and required by most other functions. Must be passed to `pm_cleanup()` to release resources.

**Enums:**

- `pm_error_t`: Integer error codes. `PM_SUCCESS` (0) indicates success. Negative values indicate errors. See `pm_error_string()` to get descriptions.
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
- `pm_sensor_type_t`: Identifies the type of power sensor.
  - `PM_SENSOR_TYPE_UNKNOWN = 0`
  - `PM_SENSOR_TYPE_I2C = 1` (e.g., INA3221)
  - `PM_SENSOR_TYPE_SYSTEM = 2` (e.g., sysfs power supply class)

**Data Structures:**

- `pm_sensor_data_t`: Holds *instantaneous* data for a single sensor.
  - `char name[64]`: Null-terminated sensor name.
  - `pm_sensor_type_t type`: Sensor type.
  - `double voltage`, `current`, `power`: Measured values (V, A, W).
  - `bool online`: Indicates if the sensor is currently readable.
  - `char status[32]`: Null-terminated status string (e.g., "OK").
  - `double warning_threshold`, `critical_threshold`: Power thresholds (W).
- `pm_stats_t`: Holds basic statistics for a metric.
  - `double min`, `max`, `avg`: Min, Max, Average values.
  - `double total`: Sum of values (can be used to calculate energy for power: Energy = Avg Power * Duration).
  - `uint64_t count`: Number of samples collected.
- `pm_sensor_stats_t`: Holds statistics for a single sensor.
  - `char name[64]`: Null-terminated sensor name.
  - `pm_stats_t voltage`, `current`, `power`: Statistics for each metric.
- `pm_power_data_t`: Structure filled by `pm_get_latest_data`.
  - `pm_sensor_data_t total`: Aggregated instantaneous data.
  - `pm_sensor_data_t* sensors`: Pointer to an array of individual sensor data. **Memory is managed by the library.** The pointer is valid until the next relevant library call or `pm_cleanup`. Do not free this pointer.
  - `int sensor_count`: Number of valid elements in the `sensors` array.
- `pm_power_stats_t`: Structure filled by `pm_get_statistics`.
  - `pm_sensor_stats_t total`: Aggregated statistics.
  - `pm_sensor_stats_t* sensors`: Pointer to an array of individual sensor statistics. **Memory is managed by the library.** The pointer is valid until the next relevant library call or `pm_cleanup`. Do not free this pointer.
  - `int sensor_count`: Number of valid elements in the `sensors` array.

**Core Functions:**

- `pm_error_t pm_init(pm_handle_t* handle)`:
  - Initializes the library, discovers sensors, allocates resources.
  - Stores the opaque library instance handle at the address provided by `handle`.
  - **Must be called first.** Returns `PM_SUCCESS` on success.
- `pm_error_t pm_cleanup(pm_handle_t handle)`:
  - Stops sampling (if active) and frees all resources associated with the `handle`.
  - **Must be called** when finished with the library to prevent resource leaks.
- `const char* pm_error_string(pm_error_t error)`:
  - Returns a constant, human-readable string describing the given error code. Do not modify or free the returned string.

**Sampling Control & Status:**

- `pm_error_t pm_set_sampling_frequency(pm_handle_t handle, int frequency_hz)`:
  - Sets the target sampling frequency (in Hz) for the background monitoring thread. Must be > 0.
- `pm_error_t pm_get_sampling_frequency(pm_handle_t handle, int* frequency_hz)`:
  - Retrieves the currently configured sampling frequency, storing it at the address `frequency_hz`.
- `pm_error_t pm_start_sampling(pm_handle_t handle)`:
  - Starts the background sampling thread. Statistics begin accumulating. Returns `PM_ERROR_ALREADY_RUNNING` if already started.
- `pm_error_t pm_stop_sampling(pm_handle_t handle)`:
  - Stops the background sampling thread. Returns `PM_ERROR_NOT_RUNNING` if not running.
- `pm_error_t pm_is_sampling(pm_handle_t handle, bool* is_sampling)`:
  - Checks if the background sampling thread is active, storing the result (`true` or `false`) at the address `is_sampling`.

**Data & Statistics Retrieval:**

- `pm_error_t pm_get_latest_data(pm_handle_t handle, pm_power_data_t* data)`:
  - Fills the user-provided `data` structure with the most recent instantaneous sensor readings.
  - The `data->sensors` pointer will point to an internal library buffer.
- `pm_error_t pm_get_statistics(pm_handle_t handle, pm_power_stats_t* stats)`:
  - Fills the user-provided `stats` structure with statistics accumulated since the last reset.
  - The `stats->sensors` pointer will point to an internal library buffer.
- `pm_error_t pm_reset_statistics(pm_handle_t handle)`:
  - Resets all accumulated statistics (min, max, avg, total, count) to zero.

**Sensor Information:**

- `pm_error_t pm_get_sensor_count(pm_handle_t handle, int* count)`:
  - Gets the total number of sensors detected by the library.
- `pm_error_t pm_get_sensor_names(pm_handle_t handle, char** names, int* count)`:
  - Fills a **caller-allocated** array of C strings (`char* names[]`) with the names of detected sensors.
  - `names`: Pointer to an array of `char*`. The caller must allocate this array. Each `char*` in the array must also point to a caller-allocated buffer (e.g., `char name_buffer[64]`) large enough to hold a sensor name.
  - `count`: `[inout]` parameter. On input, points to the allocated size of the `names` array. On output, points to the actual number of names written.
  - **Note:** This function requires careful memory management by the caller. Accessing names via `pm_get_latest_data` or `pm_get_statistics` (using the `sensors[i].name` field) is often simpler as the library manages those strings.

</details>

### C++ Bindings

<details>
<summary><strong>API Reference (C++ Wrapper)</strong></summary>

**Namespace:** `jetpwmon`

**Main Class:** `PowerMonitor`

- **Description:** An RAII wrapper class for managing the `jetpwmon` C library. It handles initialization (`pm_init`) in its constructor and cleanup (`pm_cleanup`) in its destructor automatically. It converts C API error codes into `std::runtime_error` exceptions.
- **Resource Management:** Non-copyable, but movable. Uses `std::unique_ptr` with a custom deleter for the C handle (`pm_handle_t`).
- **Constructor:** `PowerMonitor()`
  - Initializes the library connection.
  - **Throws:** `std::runtime_error` if `pm_init` fails. The exception's `what()` message contains the error description from `pm_error_string`.
- **Destructor:** `~PowerMonitor()`
  - Automatically calls `pm_cleanup` on the managed C handle.
- **Methods:**
  - `void setSamplingFrequency(int frequency_hz)`
    - Sets the background sampling frequency (Hz).
    - **Throws:** `std::runtime_error` on C API failure.
  - `int getSamplingFrequency() const`
    - Gets the current sampling frequency (Hz).
    - **Throws:** `std::runtime_error` on C API failure.
  - `void startSampling()`
    - Starts the background sampling thread.
    - **Throws:** `std::runtime_error` on C API failure (e.g., already running).
  - `void stopSampling()`
    - Stops the background sampling thread.
    - **Throws:** `std::runtime_error` on C API failure (e.g., not running).
  - `bool isSampling() const`
    - Checks if sampling is currently active.
    - **Throws:** `std::runtime_error` on C API failure.
  - `PowerData getLatestData() const`
    - Gets the most recent instantaneous sensor readings.
    - **Returns:** A `PowerData` object containing the snapshot.
    - **Throws:** `std::runtime_error` on C API failure.
    - **Note:** See `PowerData` description and Safety Notes regarding pointer validity.
  - `PowerStats getStatistics() const`
    - Gets the statistics accumulated since the last reset.
    - **Returns:** A `PowerStats` object containing the statistics.
    - **Throws:** `std::runtime_error` on C API failure.
    - **Note:** See `PowerStats` description and Safety Notes regarding pointer validity.
  - `void resetStatistics()`
    - Resets all internal accumulated statistics.
    - **Throws:** `std::runtime_error` on C API failure.
  - `int getSensorCount() const`
    - Gets the number of detected sensors.
    - **Throws:** `std::runtime_error` on C API failure.
  - `std::vector<std::string> getSensorNames() const`
    - Gets the names of all detected sensors. Handles C memory management and string conversion.
    - **Returns:** A `std::vector<std::string>` containing the sensor names.
    - **Throws:** `std::runtime_error` on C API failure.

**Data Wrapper Classes:**

- `PowerData` / `PowerStats`
  - **Description:** Thin wrappers around the C structs `pm_power_data_t` and `pm_power_stats_t`, primarily returned by `getLatestData` and `getStatistics`. They are non-copyable but movable.
  - **Memory:** They hold a *copy* of the `total` C struct member and the *raw C pointer* (`sensors`) along with the `sensor_count`. **They do NOT manage the memory pointed to by the `sensors` pointer.** That memory is owned by the underlying C library.
  - **Getters:**
    - `const pm_sensor_data_t& getTotal() const` (for `PowerData`)
    - `const pm_sensor_stats_t& getTotal() const` (for `PowerStats`)
      - Returns a const reference to the copied `total` data/statistics struct.
    - `const pm_sensor_data_t* getSensors() const` (for `PowerData`)
    - `const pm_sensor_stats_t* getSensors() const` (for `PowerStats`)
      - Returns the raw C pointer to the array of per-sensor data/statistics. **See Safety Notes.**
    - `int getSensorCount() const`: Returns the number of elements pointed to by `getSensors()`.

**Underlying C Structs:**

- The C++ wrapper provides access to data via the C structs (`pm_sensor_data_t`, `pm_stats_t`, `pm_sensor_stats_t`). Refer to the C API documentation for detailed field descriptions within these structs.

</details>

<br/>

<details>
<summary><strong>C++ Wrapper Safety & Pointer Notes</strong></summary>

- **RAII & Exceptions:** The `PowerMonitor` class significantly improves safety by automating resource cleanup (`pm_cleanup`) through its destructor (RAII) and by converting C error codes into C++ exceptions (`std::runtime_error`). Always use `try...catch` blocks when interacting with `PowerMonitor` methods.
- **Pointer Validity (`getSensors()`):** The `PowerData` and `PowerStats` objects returned by `getLatestData()` and `getStatistics()` contain raw C pointers to arrays (`sensors`). **Crucially, the C++ wrapper classes (`PowerData`, `PowerStats`) do NOT manage the lifetime of the memory these pointers point to.** This memory is managed by the C library.
  - **Assumption:** The memory pointed to by `getSensors()` is typically valid only **temporarily**, likely until the next non-const call to the `PowerMonitor` object or until the `PowerMonitor` object is destroyed.
  - **Guideline:** Access the data through the pointer returned by `getSensors()` **immediately** after the call to `getLatestData()` or `getStatistics()`, within the same scope. Do **not** store this raw pointer for later use, as it may become invalid (dangling pointer).
- **C String Handling:** Data structures contain C-style fixed-size character arrays (e.g., `name[64]`, `status[32]`). Use safe methods (like the `c_char_to_string` helper in the examples using `strnlen`) to convert these to `std::string` to avoid buffer over-reads, especially if null termination is not guaranteed within the fixed size.

</details>

## Building from Source

### Prerequisites

- CMake 3.10 or higher
- C++ compiler with C++17 support
- Python 3.8 or higher (for Python bindings)
- Rust toolchain (for Rust bindings)

### Build Steps

#### C Library and C++ Bindings

```bash
git clone https://github.com/nerdneilsfield/jetson-power-monitor.git
cd jetson-power-monitor
mkdir build && cd build
cmake ..
make
sudo make install
```

#### Python Bindings

```bash
python3 -m pip install setuptools pybind11
python3 -m pip install -e .

# or you need to build wheel
python3 -m pip install build
python3 -m build --wheel
# the result will be in dist/
```

#### Rust Bindings

```bash
# copy c headers and sources to rust vendor directory
make copy-rust

# build rust crate
cd bindings/rust
cargo build
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
