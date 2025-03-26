#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include "jetpwmon/jetpwmon.h"

#ifdef USE_EIGEN
#include <Eigen/Dense>
using namespace Eigen;
#endif

#define MATRIX_SIZE 1000
#define NUM_THREADS 4
#define NUM_ITERATIONS 100

/* Function declarations */
void *matrix_multiply_thread(void *arg);
double get_time_ms(void);

/* Thread arguments structure */
struct thread_args {
    int thread_id;
    int matrix_size;
    int num_iterations;
};

/* Matrix multiplication function using Eigen */
void matrix_multiply_eigen(int size, int num_iterations) {
    #ifdef USE_EIGEN
    MatrixXd A = MatrixXd::Random(size, size);
    MatrixXd B = MatrixXd::Random(size, size);
    MatrixXd C(size, size);

    for (int i = 0; i < num_iterations; i++) {
        C = A * B;
        A = C;  // Use result as input for next iteration
    }
    #endif
}

/* Matrix multiplication function using OpenMP */
void matrix_multiply_openmp(int size, int num_iterations) {
    #ifdef _OPENMP
    double **A = (double **)malloc(size * sizeof(double *));
    double **B = (double **)malloc(size * sizeof(double *));
    double **C = (double **)malloc(size * sizeof(double *));

    for (int i = 0; i < size; i++) {
        A[i] = (double *)malloc(size * sizeof(double));
        B[i] = (double *)malloc(size * sizeof(double));
        C[i] = (double *)malloc(size * sizeof(double));
    }

    // Initialize matrices
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            A[i][j] = rand() / (double)RAND_MAX;
            B[i][j] = rand() / (double)RAND_MAX;
        }
    }

    // Perform matrix multiplication
    for (int iter = 0; iter < num_iterations; iter++) {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                C[i][j] = 0.0;
                for (int k = 0; k < size; k++) {
                    C[i][j] += A[i][k] * B[k][j];
                }
            }
        }

        // Use result as input for next iteration
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                A[i][j] = C[i][j];
            }
        }
    }

    // Free memory
    for (int i = 0; i < size; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);
    #endif
}

/* Thread function for matrix multiplication */
void *matrix_multiply_thread(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    
    #ifdef USE_EIGEN
    matrix_multiply_eigen(args->matrix_size, args->num_iterations);
    #else
    matrix_multiply_openmp(args->matrix_size, args->num_iterations);
    #endif

    return NULL;
}

/* Get current time in milliseconds */
double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int main(void) {
    pm_handle_t handle;
    pm_error_t error;
    pm_power_stats_t stats;
    pthread_t threads[NUM_THREADS];
    struct thread_args thread_args[NUM_THREADS];
    double start_time, end_time;
    double total_time;

    /* Initialize the power monitor */
    error = pm_init(&handle);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Failed to initialize power monitor: %s\n", pm_error_string(error));
        return 1;
    }

    /* Start power sampling */
    error = pm_start_sampling(handle);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Failed to start power sampling: %s\n", pm_error_string(error));
        pm_cleanup(handle);
        return 1;
    }

    printf("开始功耗采样...\n");

    /* Initialize random seed */
    srand(time(NULL));

    /* Start matrix multiplication */
    printf("开始执行CPU密集型任务...\n");
    start_time = get_time_ms();

    /* Create and start threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].matrix_size = MATRIX_SIZE;
        thread_args[i].num_iterations = NUM_ITERATIONS;
        pthread_create(&threads[i], NULL, matrix_multiply_thread, &thread_args[i]);
    }

    /* Wait for all threads to complete */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    end_time = get_time_ms();
    total_time = (end_time - start_time) / 1000.0;
    printf("CPU密集型任务完成\n");
    printf("总执行时间: %.2f 秒\n", total_time);

    /* Stop power sampling */
    error = pm_stop_sampling(handle);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Failed to stop power sampling: %s\n", pm_error_string(error));
        pm_cleanup(handle);
        return 1;
    }

    /* Get power statistics */
    error = pm_get_statistics(handle, &stats);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Failed to get power statistics: %s\n", pm_error_string(error));
        pm_cleanup(handle);
        return 1;
    }

    /* Print power statistics */
    printf("\n功耗统计信息:\n");
    printf("总功耗:\n");
    printf("  最小值: %.2f W\n", stats.total.power.min);
    printf("  最大值: %.2f W\n", stats.total.power.max);
    printf("  平均值: %.2f W\n", stats.total.power.avg);
    printf("  总能耗: %.2f J\n", stats.total.power.avg * total_time);
    printf("  采样次数: %d\n", stats.total.power.count);

    printf("\n各传感器功耗信息:\n");
    for (int i = 0; i < stats.sensor_count; i++) {
        printf("\n传感器: %s\n", stats.sensors[i].name);
        printf("  最小值: %.2f W\n", stats.sensors[i].power.min);
        printf("  最大值: %.2f W\n", stats.sensors[i].power.max);
        printf("  平均值: %.2f W\n", stats.sensors[i].power.avg);
        printf("  总能耗: %.2f J\n", stats.sensors[i].power.avg * total_time);
        printf("  采样次数: %d\n", stats.sensors[i].power.count);
    }

    /* Clean up */
    pm_cleanup(handle);
    return 0;
} 