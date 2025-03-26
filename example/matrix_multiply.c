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

#ifdef USE_OMP
#include <omp.h>
#endif

/* 减小矩阵大小，避免内存问题 */
#define MATRIX_SIZE 5000
#define NUM_THREADS 4
#define NUM_ITERATIONS 10

/* Function declarations */
void *matrix_multiply_thread(void *arg);
double get_time_ms(void);

/* Thread arguments structure */
struct thread_args {
    int thread_id;
    int matrix_size;
    int num_iterations;
    bool completed;  /* 添加完成标志 */
};

/* Matrix multiplication function using Eigen */
void matrix_multiply_eigen(int size, int num_iterations) {
    #ifdef USE_EIGEN
    printf("线程开始执行Eigen矩阵乘法...\n");
    MatrixXd A = MatrixXd::Random(size, size);
    MatrixXd B = MatrixXd::Random(size, size);
    MatrixXd C(size, size);

    for (int i = 0; i < num_iterations; i++) {
        C = A * B;
        A = C;  // Use result as input for next iteration
    }
    printf("线程完成Eigen矩阵乘法\n");
    #endif
}

/* Matrix multiplication function using OpenMP */
void matrix_multiply_openmp(int size, int num_iterations) {
    #ifdef _OPENMP
    printf("线程开始执行OpenMP矩阵乘法...\n");
    double **A = (double **)malloc(size * sizeof(double *));
    double **B = (double **)malloc(size * sizeof(double *));
    double **C = (double **)malloc(size * sizeof(double *));

    if (!A || !B || !C) {
        printf("内存分配失败\n");
        return;
    }

    for (int i = 0; i < size; i++) {
        A[i] = (double *)malloc(size * sizeof(double));
        B[i] = (double *)malloc(size * sizeof(double));
        C[i] = (double *)malloc(size * sizeof(double));
        if (!A[i] || !B[i] || !C[i]) {
            printf("内存分配失败\n");
            return;
        }
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
    printf("线程完成OpenMP矩阵乘法\n");
    #endif
}

/* Thread function for matrix multiplication */
void *matrix_multiply_thread(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    
    printf("线程 %d 开始执行...\n", args->thread_id);
    
    #ifdef USE_EIGEN
    matrix_multiply_eigen(args->matrix_size, args->num_iterations);
    #else
    matrix_multiply_openmp(args->matrix_size, args->num_iterations);
    #endif

    args->completed = true;
    printf("线程 %d 完成执行\n", args->thread_id);
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
        fprintf(stderr, "初始化功耗监控失败: %s\n", pm_error_string(error));
        return 1;
    }

    /* Start power sampling */
    error = pm_start_sampling(handle);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "启动功耗采样失败: %s\n", pm_error_string(error));
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
        thread_args[i].completed = false;
        if (pthread_create(&threads[i], NULL, matrix_multiply_thread, &thread_args[i]) != 0) {
            fprintf(stderr, "创建线程 %d 失败\n", i);
            pm_cleanup(handle);
            return 1;
        }
    }

    /* Wait for all threads to complete */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "等待线程 %d 完成失败\n", i);
        }
    }

    end_time = get_time_ms();
    total_time = (end_time - start_time) / 1000.0;
    printf("CPU密集型任务完成\n");
    printf("总执行时间: %.2f 秒\n", total_time);

    /* Stop power sampling */
    error = pm_stop_sampling(handle);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "停止功耗采样失败: %s\n", pm_error_string(error));
        pm_cleanup(handle);
        return 1;
    }

    /* Get power statistics */
    error = pm_get_statistics(handle, &stats);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "获取功耗统计信息失败: %s\n", pm_error_string(error));
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
    printf("  采样次数: %ld\n", stats.total.power.count);

    printf("\n各传感器功耗信息:\n");
    for (int i = 0; i < stats.sensor_count; i++) {
        printf("\n传感器: %s\n", stats.sensors[i].name);
        printf("  最小值: %.2f W\n", stats.sensors[i].power.min);
        printf("  最大值: %.2f W\n", stats.sensors[i].power.max);
        printf("  平均值: %.2f W\n", stats.sensors[i].power.avg);
        printf("  总能耗: %.2f J\n", stats.sensors[i].power.avg * total_time);
        printf("  采样次数: %ld\n", stats.sensors[i].power.count);
    }

    /* Clean up */
    pm_cleanup(handle);
    return 0;
}