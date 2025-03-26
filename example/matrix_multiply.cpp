#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <Eigen/Dense>
#include "jetpwmon/jetpwmon.h"

using namespace Eigen;
using namespace std;

/* 减小矩阵大小，避免内存问题 */
constexpr int MATRIX_SIZE = 1000;
constexpr int NUM_THREADS = 4;
constexpr int NUM_ITERATIONS = 10;

/* Thread arguments structure */
struct ThreadArgs {
    int thread_id;
    int matrix_size;
    int num_iterations;
};

/* Get current time in milliseconds */
double get_time_ms() {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    return duration_cast<milliseconds>(now.time_since_epoch()).count();
}

/* Thread function for matrix multiplication */
void* matrix_multiply_thread(void* arg) {
    auto* args = static_cast<ThreadArgs*>(arg);
    cout << "线程 " << args->thread_id << " 开始执行...\n";

    // Eigen implementation
    MatrixXd A = MatrixXd::Random(args->matrix_size, args->matrix_size);
    MatrixXd B = MatrixXd::Random(args->matrix_size, args->matrix_size);
    MatrixXd C(args->matrix_size, args->matrix_size);

    for (int i = 0; i < args->num_iterations; i++) {
        C.noalias() = A * B;  // 使用 noalias() 避免临时变量
        A = C;  // Use result as input for next iteration
    }

    cout << "线程 " << args->thread_id << " 完成执行\n";
    return nullptr;
}

int main() {
    pm_handle_t handle;
    pm_error_t error;
    pm_power_stats_t stats;
    vector<pthread_t> threads(NUM_THREADS);
    vector<ThreadArgs> thread_args(NUM_THREADS);
    double start_time, end_time;
    double total_time;

    /* Initialize the power monitor */
    error = pm_init(&handle);
    if (error != PM_SUCCESS) {
        cerr << "初始化功耗监控失败: " << pm_error_string(error) << endl;
        return 1;
    }

    /* Start power sampling */
    error = pm_start_sampling(handle);
    if (error != PM_SUCCESS) {
        cerr << "启动功耗采样失败: " << pm_error_string(error) << endl;
        pm_cleanup(handle);
        return 1;
    }

    cout << "开始功耗采样...\n";

    /* Start matrix multiplication */
    cout << "开始执行CPU密集型任务...\n";
    start_time = get_time_ms();

    /* Create and start threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].matrix_size = MATRIX_SIZE;
        thread_args[i].num_iterations = NUM_ITERATIONS;
        
        if (pthread_create(&threads[i], nullptr, matrix_multiply_thread, &thread_args[i]) != 0) {
            cerr << "创建线程 " << i << " 失败\n";
            pm_cleanup(handle);
            return 1;
        }
    }

    /* Wait for all threads to complete */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], nullptr) != 0) {
            cerr << "等待线程 " << i << " 完成失败\n";
        }
    }

    end_time = get_time_ms();
    total_time = (end_time - start_time) / 1000.0;
    cout << "CPU密集型任务完成\n";
    cout << "总执行时间: " << total_time << " 秒\n";

    /* Stop power sampling */
    error = pm_stop_sampling(handle);
    if (error != PM_SUCCESS) {
        cerr << "停止功耗采样失败: " << pm_error_string(error) << endl;
        pm_cleanup(handle);
        return 1;
    }

    /* Get power statistics */
    error = pm_get_statistics(handle, &stats);
    if (error != PM_SUCCESS) {
        cerr << "获取功耗统计信息失败: " << pm_error_string(error) << endl;
        pm_cleanup(handle);
        return 1;
    }

    /* Print power statistics */
    cout << "\n功耗统计信息:\n";
    cout << "总功耗:\n";
    cout << "  最小值: " << stats.total.power.min << " W\n";
    cout << "  最大值: " << stats.total.power.max << " W\n";
    cout << "  平均值: " << stats.total.power.avg << " W\n";
    cout << "  总能耗: " << stats.total.power.avg * total_time << " J\n";
    cout << "  采样次数: " << stats.total.power.count << "\n";

    cout << "\n各传感器功耗信息:\n";
    for (int i = 0; i < stats.sensor_count; i++) {
        cout << "\n传感器: " << stats.sensors[i].name << "\n";
        cout << "  最小值: " << stats.sensors[i].power.min << " W\n";
        cout << "  最大值: " << stats.sensors[i].power.max << " W\n";
        cout << "  平均值: " << stats.sensors[i].power.avg << " W\n";
        cout << "  总能耗: " << stats.sensors[i].power.avg * total_time << " J\n";
        cout << "  采样次数: " << stats.sensors[i].power.count << "\n";
    }

    /* Clean up */
    pm_cleanup(handle);
    return 0;
} 