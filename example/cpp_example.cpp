/**
 * @file cpp_example.cpp
 * @brief Example usage of the C++ RAII wrapper for jetpwmon
 */

#include <jetpwmon/jetpwmon++.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <functional>
#include <thread>

/**
 * @brief CPU-intensive task: Matrix multiplication
 */
void cpuIntensiveTask() {
    std::cout << "Starting CPU-intensive task..." << std::endl;
    
    // Set up random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    // Create large matrices
    const int size = 2000;
    std::vector<double> matrix1(size * size);
    std::vector<double> matrix2(size * size);
    std::vector<double> result(size * size);
    
    // Initialize matrices
    for (int i = 0; i < size * size; i++) {
        matrix1[i] = dis(gen);
        matrix2[i] = dis(gen);
    }
    
    // Perform matrix multiplication
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            result[i * size + j] = 0.0;
            for (int k = 0; k < size; k++) {
                result[i * size + j] += matrix1[i * size + k] * matrix2[k * size + j];
            }
        }
    }
    
    std::cout << "CPU-intensive task completed" << std::endl;
}

/**
 * @brief Monitor power consumption during task execution
 * @param task Task function to execute
 */
void monitorPowerConsumption(std::function<void()> task) {
    try {
        // Create PowerMonitor instance (RAII automatically manages resources)
        jetpwmon::PowerMonitor monitor;
        
        // Set sampling frequency to 1000Hz
        monitor.setSamplingFrequency(1000);
        
        // Reset statistics
        monitor.resetStatistics();
        
        // Start sampling
        std::cout << "Starting power sampling..." << std::endl;
        monitor.startSampling();
        
        // Execute task
        task();
        
        // Wait a short time to ensure complete data collection
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Stop sampling
        monitor.stopSampling();
        
        // Get statistics (RAII automatically manages memory)
        auto stats = monitor.getStatistics();
        
        // Print total power statistics
        std::cout << "\nPower Statistics:" << std::endl;
        std::cout << "Total Power:" << std::endl;
        std::cout << "  Min: " << stats.getTotal().power.min << " W" << std::endl;
        std::cout << "  Max: " << stats.getTotal().power.max << " W" << std::endl;
        std::cout << "  Avg: " << stats.getTotal().power.avg << " W" << std::endl;
        std::cout << "  Total Energy: " << stats.getTotal().power.total << " J" << std::endl;
        std::cout << "  Samples: " << stats.getTotal().power.count << std::endl;
        
        // Print individual sensor statistics
        std::cout << "\nIndividual Sensor Statistics:" << std::endl;
        for (int i = 0; i < stats.getSensorCount(); i++) {
            const auto& sensor = stats.getSensors()[i];
            std::cout << "\nSensor: " << sensor.name << std::endl;
            std::cout << "  Min: " << sensor.power.min << " W" << std::endl;
            std::cout << "  Max: " << sensor.power.max << " W" << std::endl;
            std::cout << "  Avg: " << sensor.power.avg << " W" << std::endl;
            std::cout << "  Total Energy: " << sensor.power.total << " J" << std::endl;
            std::cout << "  Samples: " << sensor.power.count << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Jetson Power Monitor C++ Example" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Monitor power consumption of CPU-intensive task
    monitorPowerConsumption(cpuIntensiveTask);
    
    return 0;
}
