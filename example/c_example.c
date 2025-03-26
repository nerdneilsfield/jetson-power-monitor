#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "jetpwmon/jetpwmon.h"

void cpu_intensive_task() {
    printf("开始执行CPU密集型任务...\n");
    
    // 模拟CPU密集型任务
    const int size = 2000;
    double *matrix1 = malloc(size * size * sizeof(double));
    double *matrix2 = malloc(size * size * sizeof(double));
    double *result = malloc(size * size * sizeof(double));
    
    // 初始化矩阵
    for (int i = 0; i < size * size; i++) {
        matrix1[i] = (double)rand() / RAND_MAX;
        matrix2[i] = (double)rand() / RAND_MAX;
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
    
    free(matrix1);
    free(matrix2);
    free(result);
    
    printf("CPU密集型任务完成\n");
}

void monitor_power_consumption(void (*task_func)()) {
    pm_handle_t handle;
    pm_error_t err;
    pm_power_stats_t stats;
    
    // 初始化功耗监控器
    err = pm_init(&handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "初始化失败: %s\n", pm_error_string(err));
        return;
    }
    
    // 设置采样频率为1000Hz
    err = pm_set_sampling_frequency(handle, 1000);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "设置采样频率失败: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // 重置统计数据
    err = pm_reset_statistics(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "重置统计数据失败: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // 开始采样
    printf("开始功耗采样...\n");
    err = pm_start_sampling(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "开始采样失败: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // 执行任务
    task_func();
    
    // 等待一小段时间确保数据采集完整
    usleep(500000);
    
    // 停止采样
    err = pm_stop_sampling(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "停止采样失败: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // 获取统计数据
    err = pm_get_statistics(handle, &stats);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "获取统计数据失败: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // 打印总功耗统计信息
    printf("\n功耗统计信息:\n");
    printf("总功耗:\n");
    printf("  最小值: %.2f W\n", stats.total.power.min);
    printf("  最大值: %.2f W\n", stats.total.power.max);
    printf("  平均值: %.2f W\n", stats.total.power.avg);
    printf("  总能耗: %.2f J\n", stats.total.power.total);
    printf("  采样次数: %lu\n", stats.total.power.count);
    
    // 打印各个传感器的功耗信息
    printf("\n各传感器功耗信息:\n");
    for (int i = 0; i < stats.sensor_count; i++) {
        printf("\n传感器: %s\n", stats.sensors[i].name);
        printf("  最小值: %.2f W\n", stats.sensors[i].power.min);
        printf("  最大值: %.2f W\n", stats.sensors[i].power.max);
        printf("  平均值: %.2f W\n", stats.sensors[i].power.avg);
        printf("  总能耗: %.2f J\n", stats.sensors[i].power.total);
        printf("  采样次数: %lu\n", stats.sensors[i].power.count);
    }
    
    // 清理资源
    pm_cleanup(handle);
}

int main() {
    printf("Jetson Power Monitor C 示例程序\n");
    printf("===============================\n");
    
    // 监控CPU密集型任务的功耗
    monitor_power_consumption(cpu_intensive_task);
    
    return 0;
}