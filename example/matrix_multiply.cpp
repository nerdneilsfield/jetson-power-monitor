#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <Eigen/Dense>
#include "jetpwmon/jetpwmon.h"

using namespace Eigen;
using namespace std;

/**
 * @brief Reduces matrix size to avoid memory issues
 */
constexpr int MATRIX_SIZE = 5000;
/**
 * @brief Number of threads to use for parallel processing
 */
constexpr int NUM_THREADS = 4;
/**
 * @brief Number of iterations for matrix multiplication
 */
constexpr int NUM_ITERATIONS = 10;

/**
 * @brief Structure to hold thread arguments
 */
struct ThreadArgs {
    int thread_id; /**< Thread identifier */
    int matrix_size; /**< Size of the matrix */
    int num_iterations; /**< Number of iterations */
};

/**
 * @brief Gets the current time in milliseconds
 * @return Current time in milliseconds
 */
double get_time_ms() {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    return duration_cast<milliseconds>(now.time_since_epoch()).count();
}

/**
 * @brief Thread function for matrix multiplication
 * @param arg Pointer to thread arguments
 */
void* matrix_multiply_thread(void* arg) {
    auto* args = static_cast<ThreadArgs*>(arg);
    cout << "Thread " << args->thread_id << " starting...\n";

    // Eigen implementation
    MatrixXd A = MatrixXd::Random(args->matrix_size, args->matrix_size);
    MatrixXd B = MatrixXd::Random(args->matrix_size, args->matrix_size);
    MatrixXd C(args->matrix_size, args->matrix_size);

    for (int i = 0; i < args->num_iterations; i++) {
        C.noalias() = A * B;  // Using noalias() to avoid temporary variables
        A = C;  // Use result as input for next iteration
    }

    cout << "Thread " << args->thread_id << " completed\n";
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
        cerr << "Failed to initialize power monitor: " << pm_error_string(error) << endl;
        return 1;
    }

    /* Start power sampling */
    error = pm_start_sampling(handle);
    if (error != PM_SUCCESS) {
        cerr << "Failed to start power sampling: " << pm_error_string(error) << endl;
        pm_cleanup(handle);
        return 1;
    }

    cout << "Starting power sampling...\n";

    /* Start matrix multiplication */
    cout << "Starting CPU-intensive task...\n";
    start_time = get_time_ms();

    /* Create and start threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].matrix_size = MATRIX_SIZE;
        thread_args[i].num_iterations = NUM_ITERATIONS;
        
        if (pthread_create(&threads[i], nullptr, matrix_multiply_thread, &thread_args[i]) != 0) {
            cerr << "Failed to create thread " << i << endl;
            pm_cleanup(handle);
            return 1;
        }
    }

    /* Wait for all threads to complete */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], nullptr) != 0) {
            cerr << "Failed to join thread " << i << endl;
        }
    }

    end_time = get_time_ms();
    total_time = (end_time - start_time) / 1000.0;
    cout << "CPU-intensive task completed\n";
    cout << "Total execution time: " << total_time << " seconds\n";

    /* Stop power sampling */
    error = pm_stop_sampling(handle);
    if (error != PM_SUCCESS) {
        cerr << "Failed to stop power sampling: " << pm_error_string(error) << endl;
        pm_cleanup(handle);
        return 1;
    }

    /* Get power statistics */
    error = pm_get_statistics(handle, &stats);
    if (error != PM_SUCCESS) {
        cerr << "Failed to get power statistics: " << pm_error_string(error) << endl;
        pm_cleanup(handle);
        return 1;
    }

    /* Print power statistics */
    cout << "\nPower Consumption Statistics:\n";
    cout << "Total Power Consumption:\n";
    cout << "  Minimum Value: " << stats.total.power.min << " W\n";
    cout << "  Maximum Value: " << stats.total.power.max << " W\n";
    cout << "  Average Value: " << stats.total.power.avg << " W\n";
    cout << "  Total Energy: " << stats.total.power.avg * total_time << " J\n";
    cout << "  Sample Count: " << stats.total.power.count << "\n";

    cout << "\nPer-Sensor Power Consumption Information:\n";
    for (int i = 0; i < stats.sensor_count; i++) {
        cout << "\nSensor: " << stats.sensors[i].name << "\n";
        cout << "  Minimum Value: " << stats.sensors[i].power.min << " W\n";
        cout << "  Maximum Value: " << stats.sensors[i].power.max << " W\n";
        cout << "  Average Value: " << stats.sensors[i].power.avg << " W\n";
        cout << "  Total Energy: " << stats.sensors[i].power.avg * total_time << " J\n";
        cout << "  Sample Count: " << stats.sensors[i].power.count << "\n";
    }

    /* Clean up */
    pm_cleanup(handle);
    return 0;
} 