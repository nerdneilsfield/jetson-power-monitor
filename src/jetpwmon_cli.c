/**
 * @file jetpwmon_cli.c
 * @brief Command line interface for the power monitor library
 *
 * This program provides a command line interface to the power monitor library
 * to monitor power consumption and collect statistics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "jetpwmon/jetpwmon.h"

/* Global handle for signal handler */
static pm_handle_t g_handle = NULL;

/* Signal handler for clean shutdown */
static void signal_handler(int signum) {
    if (g_handle) {
        printf("Stopping power monitoring...\n");
        pm_stop_sampling(g_handle);
        pm_cleanup(g_handle);
    }
    exit(0);
}

/* Print power data */
static void print_power_data(const pm_power_data_t* data) {
    printf("===== Power Data =====\n");
    printf("Total Power: %.2f W (%.2f V, %.2f A)\n",
           data->total.power, data->total.voltage, data->total.current);
    printf("\nSensor Details:\n");

    for (int i = 0; i < data->sensor_count; i++) {
        const pm_sensor_data_t* sensor = &data->sensors[i];
        printf("  %s: %.2f W (%.2f V, %.2f A) - %s\n",
               sensor->name, sensor->power, sensor->voltage, sensor->current,
               sensor->online ? "Online" : "Offline");
    }

    printf("\n");
}

/* Print power statistics */
static void print_power_stats(const pm_power_stats_t* stats) {
    printf("===== Power Statistics =====\n");
    printf("Total Power: Avg=%.2f W, Min=%.2f W, Max=%.2f W\n",
           stats->total.power.avg, stats->total.power.min, stats->total.power.max);
    printf("\nSensor Statistics:\n");

    for (int i = 0; i < stats->sensor_count; i++) {
        const pm_sensor_stats_t* sensor = &stats->sensors[i];
        printf("  %s: Avg=%.2f W, Min=%.2f W, Max=%.2f W\n",
               sensor->name, sensor->power.avg, sensor->power.min, sensor->power.max);
    }

    printf("\n");
}

/* Main function */
int main(int argc, char* argv[]) {
    pm_error_t error;
    pm_handle_t handle;
    int sampling_frequency = 1;
    int duration = 10;

    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            sampling_frequency = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            duration = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [-f frequency_hz] [-d duration_seconds]\n", argv[0]);
            printf("  -f frequency_hz      Sampling frequency in Hz (default: 1)\n");
            printf("  -d duration_seconds  Sampling duration in seconds (default: 10)\n");
            printf("  -h, --help           Show this help message\n");
            return 0;
        }
    }

    /* Set up signal handler for clean shutdown */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Initialize the library */
    printf("Initializing power monitor...\n");
    error = pm_init(&handle);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Error initializing power monitor: %s\n", pm_error_string(error));
        return 1;
    }

    /* Set the global handle for the signal handler */
    g_handle = handle;

    /* Get the number of sensors */
    int sensor_count;
    error = pm_get_sensor_count(handle, &sensor_count);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Error getting sensor count: %s\n", pm_error_string(error));
        pm_cleanup(handle);
        return 1;
    }

    printf("Found %d power sensors\n", sensor_count);

    /* Set the sampling frequency */
    printf("Setting sampling frequency to %d Hz...\n", sampling_frequency);
    error = pm_set_sampling_frequency(handle, sampling_frequency);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Error setting sampling frequency: %s\n", pm_error_string(error));
        pm_cleanup(handle);
        return 1;
    }

    /* Start sampling */
    printf("Starting power monitoring...\n");
    error = pm_start_sampling(handle);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Error starting power monitoring: %s\n", pm_error_string(error));
        pm_cleanup(handle);
        return 1;
    }

    /* Allocate memory for power data and statistics */
    pm_power_data_t data;
    data.sensors = NULL;

    pm_power_stats_t stats;
    stats.sensors = NULL;

    /* Monitor for the specified duration */
    printf("Monitoring power for %d seconds...\n", duration);
    for (int i = 0; i < duration; i++) {
        /* Sleep for 1 second */
        sleep(1);

        /* Get the latest power data */
        error = pm_get_latest_data(handle, &data);
        if (error != PM_SUCCESS) {
            fprintf(stderr, "Error getting power data: %s\n", pm_error_string(error));
            continue;
        }

        /* Print the power data */
        print_power_data(&data);

        /* Get the power statistics */
        if (i > 0 && i % 5 == 0) {
            error = pm_get_statistics(handle, &stats);
            if (error != PM_SUCCESS) {
                fprintf(stderr, "Error getting power statistics: %s\n", pm_error_string(error));
                continue;
            }

            /* Print the power statistics */
            print_power_stats(&stats);

            /* Reset the statistics if this is half-way through */
            if (i == duration / 2) {
                printf("Resetting statistics...\n");
                error = pm_reset_statistics(handle);
                if (error != PM_SUCCESS) {
                    fprintf(stderr, "Error resetting statistics: %s\n", pm_error_string(error));
                }
            }
        }
    }

    /* Stop sampling */
    printf("Stopping power monitoring...\n");
    error = pm_stop_sampling(handle);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Error stopping power monitoring: %s\n", pm_error_string(error));
    }

    /* Get the final power statistics */
    error = pm_get_statistics(handle, &stats);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Error getting power statistics: %s\n", pm_error_string(error));
    } else {
        /* Print the power statistics */
        print_power_stats(&stats);
    }

    /* Clean up */
    printf("Cleaning up...\n");
    if (data.sensors) free(data.sensors);
    if (stats.sensors) free(stats.sensors);

    error = pm_cleanup(handle);
    if (error != PM_SUCCESS) {
        fprintf(stderr, "Error cleaning up: %s\n", pm_error_string(error));
        return 1;
    }

    printf("Power monitoring completed successfully.\n");
    return 0;
}