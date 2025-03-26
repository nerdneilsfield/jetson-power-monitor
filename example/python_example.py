#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import jetpwmon
import time
import numpy as np

def cpu_intensive_task():
    """Simulate a CPU-intensive task"""
    print("Starting CPU-intensive task...")
    # Create a large matrix and perform matrix operations
    size = 10000
    matrix1 = np.random.rand(size, size)
    matrix2 = np.random.rand(size, size)
    
    # Perform matrix multiplication
    result = np.dot(matrix1, matrix2)
    print("CPU-intensive task completed")

def monitor_power_consumption(task_func):
    """Monitor power consumption during task execution"""
    # Create a power monitor instance
    monitor = jetpwmon.PowerMonitor()
    
    # Set the sampling frequency to 1000Hz
    monitor.set_sampling_frequency(1000)
    
    # Reset statistics
    monitor.reset_statistics()
    
    # Start sampling
    print("Starting power sampling...")
    monitor.start_sampling()
    
    # Execute the task
    task_func()
    
    # Wait for a short period to ensure data collection is complete
    time.sleep(0.5)
    
    # Stop sampling
    monitor.stop_sampling()
    
    # Get statistics
    stats = monitor.get_statistics()
    
    # Print total power consumption statistics
    total_stats = stats['total']
    print("\nPower Consumption Statistics:")
    print(f"Total Power Consumption:")
    print(f"  Minimum Value: {total_stats['power']['min']:.2f} W")
    print(f"  Maximum Value: {total_stats['power']['max']:.2f} W")
    print(f"  Average Value: {total_stats['power']['avg']:.2f} W")
    print(f"  Total Energy Consumption: {total_stats['power']['total']:.2f} J")
    print(f"  Sample Count: {total_stats['power']['count']}")
    
    # Print power consumption information for each sensor
    print("\nPower Consumption Information for Each Sensor:")
    for sensor in stats['sensors']:
        print(f"\nSensor: {sensor['name']}")
        print(f"  Minimum Value: {sensor['power']['min']:.2f} W")
        print(f"  Maximum Value: {sensor['power']['max']:.2f} W")
        print(f"  Average Value: {sensor['power']['avg']:.2f} W")
        print(f"  Total Energy Consumption: {sensor['power']['total']:.2f} J")
        print(f"  Sample Count: {sensor['power']['count']}")

def main():
    """Main function"""
    print("Jetson Power Monitor Example Program")
    print("=============================")
    
    # Monitor power consumption for CPU-intensive task
    monitor_power_consumption(cpu_intensive_task)

if __name__ == "__main__":
    main()
