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
 * @brief CPU密集型任务：矩阵乘法
 */
void cpuIntensiveTask() {
    std::cout << "开始执行CPU密集型任务..." << std::endl;
    
    // 设置随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    // 创建大矩阵
    const int size = 2000;
    std::vector<double> matrix1(size * size);
    std::vector<double> matrix2(size * size);
    std::vector<double> result(size * size);
    
    // 初始化矩阵
    for (int i = 0; i < size * size; i++) {
        matrix1[i] = dis(gen);
        matrix2[i] = dis(gen);
    }
    
    // 执行矩阵乘法
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            result[i * size + j] = 0.0;
            for (int k = 0; k < size; k++) {
                result[i * size + j] += matrix1[i * size + k] * matrix2[k * size + j];
            }
        }
    }
    
    std::cout << "CPU密集型任务完成" << std::endl;
}

/**
 * @brief 监控任务执行期间的功耗
 * @param task 要执行的任务函数
 */
void monitorPowerConsumption(std::function<void()> task) {
    try {
        // 创建 PowerMonitor 实例（RAII 自动管理资源）
        jetpwmon::PowerMonitor monitor;
        
        // 设置采样频率为 1000Hz
        monitor.setSamplingFrequency(1000);
        
        // 重置统计数据
        monitor.resetStatistics();
        
        // 开始采样
        std::cout << "开始功耗采样..." << std::endl;
        monitor.startSampling();
        
        // 执行任务
        task();
        
        // 等待一小段时间确保数据采集完整
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 停止采样
        monitor.stopSampling();
        
        // 获取统计数据
        auto stats = monitor.getStatistics();
        
        // 打印总功耗统计信息
        std::cout << "\n功耗统计信息:" << std::endl;
        std::cout << "总功耗:" << std::endl;
        std::cout << "  最小值: " << stats.total.power.min << " W" << std::endl;
        std::cout << "  最大值: " << stats.total.power.max << " W" << std::endl;
        std::cout << "  平均值: " << stats.total.power.avg << " W" << std::endl;
        std::cout << "  总能耗: " << stats.total.power.total << " J" << std::endl;
        std::cout << "  采样次数: " << stats.total.power.count << std::endl;
        
        // 打印各个传感器的功耗信息
        std::cout << "\n各传感器功耗信息:" << std::endl;
        for (int i = 0; i < stats.sensor_count; i++) {
            std::cout << "\n传感器: " << stats.sensors[i].name << std::endl;
            std::cout << "  最小值: " << stats.sensors[i].power.min << " W" << std::endl;
            std::cout << "  最大值: " << stats.sensors[i].power.max << " W" << std::endl;
            std::cout << "  平均值: " << stats.sensors[i].power.avg << " W" << std::endl;
            std::cout << "  总能耗: " << stats.sensors[i].power.total << " J" << std::endl;
            std::cout << "  采样次数: " << stats.sensors[i].power.count << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Jetson Power Monitor C++ 示例程序" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // 监控CPU密集型任务的功耗
    monitorPowerConsumption(cpuIntensiveTask);
    
    return 0;
}
