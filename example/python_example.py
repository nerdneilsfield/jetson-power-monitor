#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import jetpwmon
import time
import numpy as np

def cpu_intensive_task():
    """模拟一个CPU密集型任务"""
    print("开始执行CPU密集型任务...")
    # 创建一个大型矩阵并进行矩阵运算
    size = 10000
    matrix1 = np.random.rand(size, size)
    matrix2 = np.random.rand(size, size)
    
    # 执行矩阵乘法
    result = np.dot(matrix1, matrix2)
    print("CPU密集型任务完成")

def monitor_power_consumption(task_func):
    """监控任务执行期间的功耗"""
    # 创建功耗监控器实例
    monitor = jetpwmon.PowerMonitor()
    
    # 设置采样频率为1000Hz
    monitor.set_sampling_frequency(1000)
    
    # 重置统计数据
    monitor.reset_statistics()
    
    # 开始采样
    print("开始功耗采样...")
    monitor.start_sampling()
    
    # 执行任务
    task_func()
    
    # 等待一小段时间确保数据采集完整
    time.sleep(0.5)
    
    # 停止采样
    monitor.stop_sampling()
    
    # 获取统计数据
    stats = monitor.get_statistics()
    
    # 打印总功耗统计信息
    total_stats = stats['total']
    print("\n功耗统计信息:")
    print(f"总功耗:")
    print(f"  最小值: {total_stats['power']['min']:.2f} W")
    print(f"  最大值: {total_stats['power']['max']:.2f} W")
    print(f"  平均值: {total_stats['power']['avg']:.2f} W")
    print(f"  总能耗: {total_stats['power']['total']:.2f} J")
    print(f"  采样次数: {total_stats['power']['count']}")
    
    # 打印各个传感器的功耗信息
    print("\n各传感器功耗信息:")
    for sensor in stats['sensors']:
        print(f"\n传感器: {sensor['name']}")
        print(f"  最小值: {sensor['power']['min']:.2f} W")
        print(f"  最大值: {sensor['power']['max']:.2f} W")
        print(f"  平均值: {sensor['power']['avg']:.2f} W")
        print(f"  总能耗: {sensor['power']['total']:.2f} J")
        print(f"  采样次数: {sensor['power']['count']}")

def main():
    """主函数"""
    print("Jetson Power Monitor 示例程序")
    print("=============================")
    
    # 监控CPU密集型任务的功耗
    monitor_power_consumption(cpu_intensive_task)

if __name__ == "__main__":
    main()
