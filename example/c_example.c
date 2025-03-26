#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <omp.h>
#include "jetpwmon/jetpwmon.h"

void cpu_intensive_task() {
    printf("Starting CPU-intensive task...\n");
    
    // Simulate CPU-intensive task
    const int size = 2000;
    double *matrix1 = malloc(size * size * sizeof(double));
    double *matrix2 = malloc(size * size * sizeof(double));
    double *result = malloc(size * size * sizeof(double));
    
    // Initialize matrices
    #pragma omp parallel for
    for (int i = 0; i < size * size; i++) {
        matrix1[i] = (double)rand() / RAND_MAX;
        matrix2[i] = (double)rand() / RAND_MAX;
    }
    
    // Perform matrix multiplication
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
    
    free(matrix1);
    free(matrix2);
    free(result);
    
    printf("CPU-intensive task completed\n");
}

void monitor_power_consumption(void (*task_func)()) {
    pm_handle_t handle;
    pm_error_t err;
    pm_power_stats_t stats;
    
    // Initialize power monitor
    err = pm_init(&handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "Initialization failed: %s\n", pm_error_string(err));
        return;
    }
    
    // Set sampling frequency to 1000Hz
    err = pm_set_sampling_frequency(handle, 1000);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "Failed to set sampling frequency: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // Reset statistics
    err = pm_reset_statistics(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "Failed to reset statistics: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // Start sampling
    printf("Starting power sampling...\n");
    err = pm_start_sampling(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "Failed to start sampling: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // Execute task
    task_func();
    
    // Wait for a short period to ensure data collection is complete
    usleep(500000);
    
    // Stop sampling
    err = pm_stop_sampling(handle);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "Failed to stop sampling: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // Get statistics
    err = pm_get_statistics(handle, &stats);
    if (err != PM_SUCCESS) {
        fprintf(stderr, "Failed to get statistics: %s\n", pm_error_string(err));
        pm_cleanup(handle);
        return;
    }
    
    // Print total power consumption statistics
    printf("\nPower Consumption Statistics:\n");
    printf("Total Power Consumption:\n");
    printf("  Minimum Value: %.2f W\n", stats.total.power.min);
    printf("  Maximum Value: %.2f W\n", stats.total.power.max);
    printf("  Average Value: %.2f W\n", stats.total.power.avg);
    printf("  Total Energy Consumption: %.2f J\n", stats.total.power.total);
    printf("  Sample Count: %lu\n", stats.total.power.count);
    
    // Print power consumption information for each sensor
    printf("\nPower Consumption Information for Each Sensor:\n");
    for (int i = 0; i < stats.sensor_count; i++) {
        printf("\nSensor: %s\n", stats.sensors[i].name);
        printf("  Minimum Value: %.2f W\n", stats.sensors[i].power.min);
        printf("  Maximum Value: %.2f W\n", stats.sensors[i].power.max);
        printf("  Average Value: %.2f W\n", stats.sensors[i].power.avg);
        printf("  Total Energy Consumption: %.2f J\n", stats.sensors[i].power.total);
        printf("  Sample Count: %lu\n", stats.sensors[i].power.count);
    }
    
    // Cleanup resources
    pm_cleanup(handle);
}

int main() {
    printf("Jetson Power Monitor C Example Program\n");
    printf("===============================\n");
    
    // Monitor power consumption for CPU-intensive task
    monitor_power_consumption(cpu_intensive_task);
    
    return 0;
}