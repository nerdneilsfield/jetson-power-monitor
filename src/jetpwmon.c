/*
 * @file jetpwmon.c
 * @author Qi Deng<dengqi935@gmail.com>
 * @brief Implementation of the power monitor library
 */

/* Define _POSIX_C_SOURCE to enable POSIX extensions including CLOCK_REALTIME */
#define _POSIX_C_SOURCE 200809L
/* Define _XOPEN_SOURCE for usleep */
#define _XOPEN_SOURCE 500

#include "jetpwmon/jetpwmon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  /* For access() and usleep() */
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Default configuration */
#define DEFAULT_SAMPLING_FREQUENCY_HZ 1

/* Paths for power sensors */
#define I2C_PATH "/sys/bus/i2c/devices"
#define POWER_SUPPLY_PATH "/sys/class/power_supply"

/* Testing mode environment variable */
#define ENV_JTOP_TESTING "JTOP_TESTING"

/* Internal structure for the library handle */
struct pm_handle_s
{
        bool initialized;          /* Whether the library is initialized */
        bool sampling;             /* Whether sampling is active */
        int sampling_frequency_hz; /* Sampling frequency in Hz */

        /* Sampling thread */
        pthread_t sampling_thread;  /* Thread ID */
        pthread_mutex_t data_mutex; /* Mutex for data access */
        bool thread_stop_flag;      /* Flag to stop the thread */

        /* Sensor information */
        char **sensor_paths;            /* Array of sensor paths */
        char **sensor_names;            /* Array of sensor names */
        pm_sensor_type_t *sensor_types; /* Array of sensor types */
        int sensor_count;               /* Number of sensors */

        /* Current data */
        pm_power_data_t latest_data; /* Latest power data */

        /* Statistics */
        pm_power_stats_t statistics; /* Power statistics */

        /* Time tracking */
        struct timespec last_sample_time; /* Time of the last sample */

        /* Paths */
        char i2c_path[256];          /* Path to I2C devices */
        char power_supply_path[256]; /* Path to power supplies */
};

/* Forward declarations for internal functions */
static void *sampling_thread_func(void *arg);
static pm_error_t discover_sensors(pm_handle_t handle);
static pm_error_t read_sensor_data(pm_handle_t handle);
static pm_error_t update_statistics(pm_handle_t handle);
static bool check_file_exists(const char *path);
static pm_error_t find_all_i2c_power_monitor(pm_handle_t handle);
static pm_error_t find_all_system_monitor(pm_handle_t handle);
static void calculate_total_power(pm_handle_t handle);
static char *strdup_safe(const char *str);

/* Forward declarations for static functions */
static bool is_directory(const char *path);
static pm_error_t find_driver_power_folders(pm_handle_t handle, const char *path);
static pm_error_t list_all_i2c_ports(pm_handle_t handle, const char *path);

/* Error messages */
static const char *error_messages[] = {
    "Success",
    "Initialization failed",
    "Library not initialized",
    "Sampling already running",
    "Sampling not running",
    "Invalid sampling frequency",
    "No sensors found",
    "File access error",
    "Memory allocation error",
    "Thread creation error"};

/* Initialize the library */
pm_error_t pm_init(pm_handle_t *handle)
{
        if (!handle)
        {
                return PM_ERROR_INIT_FAILED;
        }

        /* Allocate memory for the handle */
        *handle = (pm_handle_t)malloc(sizeof(struct pm_handle_s));
        if (!*handle)
        {
                return PM_ERROR_MEMORY;
        }

        /* Initialize the handle */
        memset(*handle, 0, sizeof(struct pm_handle_s));
        (*handle)->sampling_frequency_hz = DEFAULT_SAMPLING_FREQUENCY_HZ;

        /* Set the paths based on environment variables */
        if (getenv(ENV_JTOP_TESTING))
        {
                snprintf((*handle)->i2c_path, sizeof((*handle)->i2c_path), "/fake_sys/bus/i2c/devices");
                snprintf((*handle)->power_supply_path, sizeof((*handle)->power_supply_path), "/fake_sys/class/power_supply");
        }
        else
        {
                snprintf((*handle)->i2c_path, sizeof((*handle)->i2c_path), "%s", I2C_PATH);
                snprintf((*handle)->power_supply_path, sizeof((*handle)->power_supply_path), "%s", POWER_SUPPLY_PATH);
        }

        /* Initialize the mutex */
        if (pthread_mutex_init(&(*handle)->data_mutex, NULL) != 0)
        {
                free(*handle);
                *handle = NULL;
                return PM_ERROR_INIT_FAILED;
        }

        /* Discover sensors */
        pm_error_t error = discover_sensors(*handle);
        if (error != PM_SUCCESS)
        {
                pthread_mutex_destroy(&(*handle)->data_mutex);
                free(*handle);
                *handle = NULL;
                return error;
        }

        /* Initialize data structures */
        (*handle)->latest_data.sensors = (pm_sensor_data_t *)malloc((*handle)->sensor_count * sizeof(pm_sensor_data_t));
        (*handle)->statistics.sensors = (pm_sensor_stats_t *)malloc((*handle)->sensor_count * sizeof(pm_sensor_stats_t));

        if (!(*handle)->latest_data.sensors || !(*handle)->statistics.sensors)
        {
                if ((*handle)->latest_data.sensors)
                        free((*handle)->latest_data.sensors);
                if ((*handle)->statistics.sensors)
                        free((*handle)->statistics.sensors);

                for (int i = 0; i < (*handle)->sensor_count; i++)
                {
                        if ((*handle)->sensor_names[i])
                                free((*handle)->sensor_names[i]);
                        if ((*handle)->sensor_paths[i])
                                free((*handle)->sensor_paths[i]);
                }

                free((*handle)->sensor_names);
                free((*handle)->sensor_paths);
                free((*handle)->sensor_types);

                pthread_mutex_destroy(&(*handle)->data_mutex);
                free(*handle);
                *handle = NULL;
                return PM_ERROR_MEMORY;
        }

        /* Initialize the data */
        memset((*handle)->latest_data.sensors, 0, (*handle)->sensor_count * sizeof(pm_sensor_data_t));
        memset((*handle)->statistics.sensors, 0, (*handle)->sensor_count * sizeof(pm_sensor_stats_t));

        for (int i = 0; i < (*handle)->sensor_count; i++)
        {
                strncpy((*handle)->latest_data.sensors[i].name, (*handle)->sensor_names[i], sizeof((*handle)->latest_data.sensors[i].name) - 1);
                (*handle)->latest_data.sensors[i].type = (*handle)->sensor_types[i];

                strncpy((*handle)->statistics.sensors[i].name, (*handle)->sensor_names[i], sizeof((*handle)->statistics.sensors[i].name) - 1);
        }

        (*handle)->initialized = true;
        return PM_SUCCESS;
}

/* Clean up resources */
pm_error_t pm_cleanup(pm_handle_t handle)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        /* Stop sampling if it's running */
        if (handle->sampling)
        {
                pm_stop_sampling(handle);
        }

        /* Free resources */
        if (handle->latest_data.sensors)
        {
                free(handle->latest_data.sensors);
        }

        if (handle->statistics.sensors)
        {
                free(handle->statistics.sensors);
        }

        if (handle->sensor_names)
        {
                for (int i = 0; i < handle->sensor_count; i++)
                {
                        if (handle->sensor_names[i])
                                free(handle->sensor_names[i]);
                }
                free(handle->sensor_names);
        }

        if (handle->sensor_paths)
        {
                for (int i = 0; i < handle->sensor_count; i++)
                {
                        if (handle->sensor_paths[i])
                                free(handle->sensor_paths[i]);
                }
                free(handle->sensor_paths);
        }

        if (handle->sensor_types)
        {
                free(handle->sensor_types);
        }

        /* Destroy the mutex */
        pthread_mutex_destroy(&handle->data_mutex);

        /* Free the handle */
        free(handle);

        return PM_SUCCESS;
}

/* Set the sampling frequency */
pm_error_t pm_set_sampling_frequency(pm_handle_t handle, int frequency_hz)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        if (frequency_hz <= 0)
        {
                return PM_ERROR_INVALID_FREQUENCY;
        }

        /* Lock the mutex to update the frequency */
        pthread_mutex_lock(&handle->data_mutex);
        handle->sampling_frequency_hz = frequency_hz;
        pthread_mutex_unlock(&handle->data_mutex);

        return PM_SUCCESS;
}

/* Get the current sampling frequency */
pm_error_t pm_get_sampling_frequency(pm_handle_t handle, int *frequency_hz)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        if (!frequency_hz)
        {
                return PM_ERROR_INIT_FAILED;
        }

        *frequency_hz = handle->sampling_frequency_hz;
        return PM_SUCCESS;
}

/* Start sampling */
pm_error_t pm_start_sampling(pm_handle_t handle)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        if (handle->sampling)
        {
                return PM_ERROR_ALREADY_RUNNING;
        }

        /* Reset the stop flag */
        handle->thread_stop_flag = false;

        /* Create the sampling thread */
        if (pthread_create(&handle->sampling_thread, NULL, sampling_thread_func, handle) != 0)
        {
                return PM_ERROR_THREAD;
        }

        handle->sampling = true;
        return PM_SUCCESS;
}

/* Stop sampling */
pm_error_t pm_stop_sampling(pm_handle_t handle)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        if (!handle->sampling)
        {
                return PM_ERROR_NOT_RUNNING;
        }

        /* Set the stop flag and wait for the thread to exit */
        handle->thread_stop_flag = true;
        pthread_join(handle->sampling_thread, NULL);

        handle->sampling = false;
        return PM_SUCCESS;
}

/* Check if sampling is active */
pm_error_t pm_is_sampling(pm_handle_t handle, bool *is_sampling)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        if (!is_sampling)
        {
                return PM_ERROR_INIT_FAILED;
        }

        *is_sampling = handle->sampling;
        return PM_SUCCESS;
}

/* Get the latest power data */
pm_error_t pm_get_latest_data(pm_handle_t handle, pm_power_data_t *data)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        if (!data)
        {
                return PM_ERROR_INIT_FAILED;
        }

        /* Lock the mutex to read the data */
        pthread_mutex_lock(&handle->data_mutex);

        /* Copy the total data */
        data->total = handle->latest_data.total;
        data->sensor_count = handle->sensor_count;

        /* Allocate memory for the sensors if needed */
        if (!data->sensors)
        {
                data->sensors = (pm_sensor_data_t *)malloc(handle->sensor_count * sizeof(pm_sensor_data_t));
                if (!data->sensors)
                {
                        pthread_mutex_unlock(&handle->data_mutex);
                        return PM_ERROR_MEMORY;
                }
        }

        /* Copy the sensor data */
        memcpy(data->sensors, handle->latest_data.sensors, handle->sensor_count * sizeof(pm_sensor_data_t));

        pthread_mutex_unlock(&handle->data_mutex);
        return PM_SUCCESS;
}

/* Get the power statistics */
pm_error_t pm_get_statistics(pm_handle_t handle, pm_power_stats_t *stats)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        if (!stats)
        {
                return PM_ERROR_INIT_FAILED;
        }

        /* Lock the mutex to read the statistics */
        pthread_mutex_lock(&handle->data_mutex);

        /* Copy the total statistics */
        stats->total = handle->statistics.total;
        stats->sensor_count = handle->sensor_count;

        /* Allocate memory for the sensors if needed */
        if (!stats->sensors)
        {
                stats->sensors = (pm_sensor_stats_t *)malloc(handle->sensor_count * sizeof(pm_sensor_stats_t));
                if (!stats->sensors)
                {
                        pthread_mutex_unlock(&handle->data_mutex);
                        return PM_ERROR_MEMORY;
                }
        }

        /* Copy the sensor statistics */
        memcpy(stats->sensors, handle->statistics.sensors, handle->sensor_count * sizeof(pm_sensor_stats_t));

        pthread_mutex_unlock(&handle->data_mutex);
        return PM_SUCCESS;
}

/* Reset the statistics */
pm_error_t pm_reset_statistics(pm_handle_t handle)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        /* Lock the mutex to reset the statistics */
        pthread_mutex_lock(&handle->data_mutex);

        /* Reset the total statistics */
        memset(&handle->statistics.total, 0, sizeof(pm_sensor_stats_t));
        strncpy(handle->statistics.total.name, "Total", sizeof(handle->statistics.total.name) - 1);

        /* Reset the sensor statistics */
        for (int i = 0; i < handle->sensor_count; i++)
        {
                memset(&handle->statistics.sensors[i], 0, sizeof(pm_sensor_stats_t));
                strncpy(handle->statistics.sensors[i].name, handle->sensor_names[i], sizeof(handle->statistics.sensors[i].name) - 1);
        }

        pthread_mutex_unlock(&handle->data_mutex);
        return PM_SUCCESS;
}

/* Get the number of sensors */
pm_error_t pm_get_sensor_count(pm_handle_t handle, int *count)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        if (!count)
        {
                return PM_ERROR_INIT_FAILED;
        }

        *count = handle->sensor_count;
        return PM_SUCCESS;
}

/* Get the sensor names */
pm_error_t pm_get_sensor_names(pm_handle_t handle, char **names, int *count)
{
        if (!handle || !handle->initialized)
        {
                return PM_ERROR_NOT_INITIALIZED;
        }

        if (!names || !count)
        {
                return PM_ERROR_INIT_FAILED;
        }

        if (*count < handle->sensor_count)
        {
                *count = handle->sensor_count;
                return PM_ERROR_MEMORY;
        }

        *count = handle->sensor_count;

        /* Copy the sensor names */
        for (int i = 0; i < handle->sensor_count; i++)
        {
                strncpy(names[i], handle->sensor_names[i], strlen(handle->sensor_names[i]) + 1);
        }

        return PM_SUCCESS;
}

/* Get the error message for an error code */
const char *pm_error_string(pm_error_t error)
{
        int index = -error;
        if (index < 0 || index >= (int)(sizeof(error_messages) / sizeof(error_messages[0])))
        {
                return "Unknown error";
        }

        return error_messages[index];
}

/* Sampling thread function */
static void *sampling_thread_func(void *arg)
{
        pm_handle_t handle = (pm_handle_t)arg;

        /* Calculate the sleep time in microseconds */
        int sleep_us = 1000000 / handle->sampling_frequency_hz;
        struct timespec sleep_time;

        while (!handle->thread_stop_flag)
        {
                /* Read the sensor data */
                read_sensor_data(handle);

                /* Update the statistics */
                update_statistics(handle);

                /* Sleep for the specified amount of time */
                sleep_time.tv_sec = sleep_us / 1000000;
                sleep_time.tv_nsec = (sleep_us % 1000000) * 1000;
                nanosleep(&sleep_time, NULL);
        }

        return NULL;
}

/* Discover sensors on the system */
static pm_error_t discover_sensors(pm_handle_t handle)
{
        pm_error_t error;

        /* Initialize sensor lists */
        handle->sensor_count = 0;
        handle->sensor_names = NULL;
        handle->sensor_paths = NULL;
        handle->sensor_types = NULL;

        /* Find I2C power monitors */
        error = find_all_i2c_power_monitor(handle);
        if (error != PM_SUCCESS)
        {
                return error;
        }

        /* Find system power monitors */
        error = find_all_system_monitor(handle);
        if (error != PM_SUCCESS)
        {
                return error;
        }

        /* Check if any sensors were found */
        if (handle->sensor_count == 0)
        {
                /* For testing purposes, add dummy sensors if none were found */
                if (getenv(ENV_JTOP_TESTING))
                {
                        /* Allocate memory for dummy sensors */
                        handle->sensor_count = 2;
                        handle->sensor_names = (char **)malloc(handle->sensor_count * sizeof(char *));
                        handle->sensor_paths = (char **)malloc(handle->sensor_count * sizeof(char *));
                        handle->sensor_types = (pm_sensor_type_t *)malloc(handle->sensor_count * sizeof(pm_sensor_type_t));

                        if (!handle->sensor_names || !handle->sensor_paths || !handle->sensor_types)
                        {
                                if (handle->sensor_names)
                                        free(handle->sensor_names);
                                if (handle->sensor_paths)
                                        free(handle->sensor_paths);
                                if (handle->sensor_types)
                                        free(handle->sensor_types);
                                handle->sensor_count = 0;
                                return PM_ERROR_MEMORY;
                        }

                        /* Set dummy sensor information */
                        handle->sensor_names[0] = strdup_safe("CPU");
                        handle->sensor_paths[0] = strdup_safe("/fake/cpu");
                        handle->sensor_types[0] = PM_SENSOR_TYPE_SYSTEM;

                        handle->sensor_names[1] = strdup_safe("GPU");
                        handle->sensor_paths[1] = strdup_safe("/fake/gpu");
                        handle->sensor_types[1] = PM_SENSOR_TYPE_SYSTEM;
                }
                else
                {
                        return PM_ERROR_NO_SENSORS;
                }
        }

        return PM_SUCCESS;
}

/* Find all I2C power monitors */
static pm_error_t find_all_i2c_power_monitor(pm_handle_t handle)
{
        DIR *dir;
        struct dirent *entry;
        char path[1024];  /* Increased from 512 */
        char name_path[1024];  /* Increased from 512 */
        char buffer[256];
        FILE *fp;

        /* Check if the I2C path exists */
        dir = opendir(handle->i2c_path);
        if (!dir)
        {
                fprintf(stderr, "Error: I2C folder %s doesn't exist\n", handle->i2c_path);
                return PM_SUCCESS; /* We return success but log error to maintain compatibility */
        }

        /* Scan all I2C devices for power sensors */
        while ((entry = readdir(dir)) != NULL)
        {
                if (entry->d_name[0] == '.')
                {
                        continue; /* Skip hidden files/directories */
                }

                /* Build full path to the device */
                if (snprintf(path, sizeof(path), "%s/%s", handle->i2c_path, entry->d_name) >= (int)sizeof(path)) {
                        continue; /* Skip if path would be truncated */
                }

                /* Check if it's a directory */
                if (is_directory(path))
                {
                        /* Check for a "name" file to identify the sensor type */
                        if (snprintf(name_path, sizeof(name_path), "%s/name", path) >= (int)sizeof(name_path)) {
                                continue; /* Skip if path would be truncated */
                        }

                        if (check_file_exists(name_path))
                        {
                                fp = fopen(name_path, "r");
                                if (fp)
                                {
                                        if (fgets(buffer, sizeof(buffer), fp))
                                        {
                                                /* Remove trailing newline */
                                                buffer[strcspn(buffer, "\n")] = 0;

                                                /* Look for ina3221 or similar power monitoring chips */
                                                if (strstr(buffer, "ina3221"))
                                                {
                                                        /* Find driver power folders */
                                                        find_driver_power_folders(handle, path);
                                                }
                                        }
                                        fclose(fp);
                                }
                        }
                }
        }

        closedir(dir);
        return PM_SUCCESS;
}

/* Find driver power folders for I2C devices */
static pm_error_t find_driver_power_folders(pm_handle_t handle, const char *path)
{
        DIR *dir;
        struct dirent *entry;
        char driver_path[1024];  /* Increased from 512 */
        char hwmon_path[1024];  /* Increased from 512 */

        dir = opendir(path);
        if (!dir)
        {
                return PM_ERROR_FILE_ACCESS;
        }

        while ((entry = readdir(dir)) != NULL)
        {
                if (entry->d_name[0] == '.')
                {
                        continue;
                }

                snprintf(driver_path, sizeof(driver_path), "%s/%s", path, entry->d_name);

                if (is_directory(driver_path))
                {
                        /* Check for hwmon directories (JP5 compatible) */
                        if (strstr(entry->d_name, "hwmon"))
                        {
                                DIR *hwmon_dir = opendir(driver_path);
                                if (hwmon_dir)
                                {
                                        struct dirent *hwmon_entry = readdir(hwmon_dir);
                                        while (hwmon_entry)
                                        {
                                                if (hwmon_entry->d_name[0] != '.')
                                                {
                                                        if (snprintf(hwmon_path, sizeof(hwmon_path), "%s/%s",
                                                                 driver_path, hwmon_entry->d_name) >= (int)sizeof(hwmon_path)) {
                                                                hwmon_entry = readdir(hwmon_dir);
                                                                continue; /* Skip if path would be truncated */
                                                        }
                                                        list_all_i2c_ports(handle, hwmon_path);
                                                        break;
                                                }
                                                hwmon_entry = readdir(hwmon_dir);
                                        }
                                        closedir(hwmon_dir);
                                }
                        }
                        /* Check for iio:device directories (JP4 or below) */
                        else if (strstr(entry->d_name, "iio:device"))
                        {
                                list_all_i2c_ports(handle, driver_path);
                        }
                }
        }

        closedir(dir);
        return PM_SUCCESS;
}

/* Find all system power monitors */
static pm_error_t find_all_system_monitor(pm_handle_t handle)
{
        DIR *dir;
        struct dirent *entry;
        char local_path[1024];  /* Increased from 512 */
        char path_type[1024];  /* Increased from 512 */
        char path_name[1024];  /* Increased from 512 */
        char buffer[256];
        FILE *fp;

        /* Check if the power supply path exists */
        dir = opendir(handle->power_supply_path);
        if (!dir)
        {
                fprintf(stderr, "Error: Power supply folder %s doesn't exist\n",
                        handle->power_supply_path);
                return PM_SUCCESS; /* Return success but log error */
        }

        /* Find all system power monitors */
        while ((entry = readdir(dir)) != NULL)
        {
                if (entry->d_name[0] == '.')
                {
                        continue;
                }

                /* Build full path to the power supply */
                snprintf(local_path, sizeof(local_path), "%s/%s",
                         handle->power_supply_path, entry->d_name);

                /* Only process directories */
                if (!is_directory(local_path))
                {
                        continue;
                }

                /* Get a clean name for the sensor */
                char *name = entry->d_name;
                if (strstr(name, "ucsi-source-psy-"))
                {
                        name = name + strlen("ucsi-source-psy-");
                }

                /* Read the type */
                if (snprintf(path_type, sizeof(path_type), "%s/type", local_path) >= (int)sizeof(path_type)) {
                        continue; /* Skip if path would be truncated */
                }
                char *type_supply = "SYSTEM";

                if (check_file_exists(path_type))
                {
                        fp = fopen(path_type, "r");
                        if (fp)
                        {
                                if (fgets(buffer, sizeof(buffer), fp))
                                {
                                        buffer[strcspn(buffer, "\n")] = 0;
                                        type_supply = strdup_safe(buffer);
                                }
                                fclose(fp);
                        }
                }

                /* Read the model name */
                if (snprintf(path_name, sizeof(path_name), "%s/model_name", local_path) >= (int)sizeof(path_name)) {
                        continue; /* Skip if path would be truncated */
                }
                char *model_name = "<EMPTY>";

                if (check_file_exists(path_name))
                {
                        fp = fopen(path_name, "r");
                        if (fp)
                        {
                                if (fgets(buffer, sizeof(buffer), fp))
                                {
                                        buffer[strcspn(buffer, "\n")] = 0;
                                        model_name = strdup_safe(buffer);
                                }
                                fclose(fp);
                        }
                }

                /* Check for required files for a power sensor */
                bool has_voltage = check_file_exists(strcat(strcpy(buffer, local_path), "/voltage_now"));
                bool has_current = check_file_exists(strcat(strcpy(buffer, local_path), "/current_now"));

                /* Only add sensor if it has both voltage and current capabilities */
                if (has_voltage && has_current)
                {
                        /* Allocate memory for sensor information */
                        int idx = handle->sensor_count;

                        /* Resize the sensor arrays */
                        handle->sensor_count++;
                        handle->sensor_names = realloc(handle->sensor_names,
                                                       handle->sensor_count * sizeof(char *));
                        handle->sensor_paths = realloc(handle->sensor_paths,
                                                       handle->sensor_count * sizeof(char *));
                        handle->sensor_types = realloc(handle->sensor_types,
                                                       handle->sensor_count * sizeof(pm_sensor_type_t));

                        if (!handle->sensor_names || !handle->sensor_paths || !handle->sensor_types)
                        {
                                fprintf(stderr, "Memory allocation error for sensor %s\n", name);
                                return PM_ERROR_MEMORY;
                        }

                        /* Store sensor information */
                        handle->sensor_names[idx] = strdup_safe(name);
                        handle->sensor_paths[idx] = strdup_safe(local_path);
                        handle->sensor_types[idx] = PM_SENSOR_TYPE_SYSTEM;

                        printf("Found power sensor: %s (type=%s, model=%s)\n",
                               name, type_supply, model_name);
                }
                else
                {
                        printf("Skipped %s: missing voltage or current capability\n", name);
                }

                /* Free resources if dynamically allocated */
                if (strcmp(type_supply, "SYSTEM") != 0)
                        free(type_supply);
                if (strcmp(model_name, "<EMPTY>") != 0)
                        free(model_name);
        }

        closedir(dir);
        return PM_SUCCESS;
}

/* Helper function to check if a path is a directory */
static bool is_directory(const char *path)
{
        struct stat statbuf;
        if (stat(path, &statbuf) != 0)
        {
                return false;
        }
        return S_ISDIR(statbuf.st_mode);
}

/* List all I2C ports for power monitoring */
static pm_error_t list_all_i2c_ports(pm_handle_t handle, const char *path)
{
        DIR *dir;
        struct dirent *entry;
        char port_path[512];
        char buffer[256];
        FILE *fp;

        dir = opendir(path);
        if (!dir)
        {
                return PM_ERROR_FILE_ACCESS;
        }

        /* Scan for label files that indicate power sensors */
        while ((entry = readdir(dir)) != NULL)
        {
                if (entry->d_name[0] == '.')
                {
                        continue;
                }

                /* Check for label files indicating power rails */
                if (strstr(entry->d_name, "_label") || strstr(entry->d_name, "rail_name_"))
                {
                        snprintf(port_path, sizeof(port_path), "%s/%s", path, entry->d_name);

                        /* Read the sensor name from the label file */
                        fp = fopen(port_path, "r");
                        if (fp)
                        {
                                if (fgets(buffer, sizeof(buffer), fp))
                                {
                                        buffer[strcspn(buffer, "\n")] = 0;

                                        /* Skip "NC" power (Orin family) */
                                        if (strstr(buffer, "NC"))
                                        {
                                                fclose(fp);
                                                continue;
                                        }

                                        /* Get port number from the file name */
                                        int port_number = -1;
                                        if (strstr(entry->d_name, "_label"))
                                        {
                                                if (sscanf(entry->d_name, "in%d_label", &port_number) != 1)
                                                {
                                                    #ifdef SHOW_ALL_DEBUG
                                                    printf("Failed to parse port number from %s\n", entry->d_name);
                                                    #endif
                                                    fclose(fp);
                                                    continue;
                                                }
                                        }
                                        else
                                        {
                                                if (sscanf(entry->d_name, "rail_name_%d", &port_number) != 1)
                                                {
                                                    #ifdef SHOW_ALL_DEBUG
                                                    printf("Failed to parse port number from %s\n", entry->d_name);
                                                    #endif
                                                    fclose(fp);
                                                    continue;
                                                }
                                        }

                                        /* Skip "sum of shunt voltages" (port 7) */
                                        if (port_number == 7)
                                        {
                                                fclose(fp);
                                                continue;
                                        }

                                        /* Check for voltage and current files */
                                        char volt_path[512], curr_path[512];
                                        bool has_volt = false, has_curr = false;

                                        /* Try both hwmon and iio formats */
                                        if (strstr(path, "hwmon"))
                                        {
                                                /* Try hwmon format first */
                                                snprintf(volt_path, sizeof(volt_path), "%s/in%d_input", path, port_number);
                                                snprintf(curr_path, sizeof(curr_path), "%s/curr%d_input", path, port_number);
                                                has_volt = check_file_exists(volt_path);
                                                has_curr = check_file_exists(curr_path);

                                                /* If not found, try alternative hwmon format */
                                                if (!has_volt || !has_curr)
                                                {
                                                        snprintf(volt_path, sizeof(volt_path), "%s/voltage%d_input", path, port_number);
                                                        snprintf(curr_path, sizeof(curr_path), "%s/current%d_input", path, port_number);
                                                        has_volt = check_file_exists(volt_path);
                                                        has_curr = check_file_exists(curr_path);
                                                }
                                        }
                                        else
                                        {
                                                /* Try iio format */
                                                snprintf(volt_path, sizeof(volt_path), "%s/in_voltage%d_input", path, port_number);
                                                snprintf(curr_path, sizeof(curr_path), "%s/in_current%d_input", path, port_number);
                                                has_volt = check_file_exists(volt_path);
                                                has_curr = check_file_exists(curr_path);
                                        }

                                        #ifdef SHOW_ALL_DEBUG
                                        printf("Checking sensor %s (port %d):\n", buffer, port_number);
                                        printf("  Voltage path: %s (exists: %d)\n", volt_path, has_volt);
                                        printf("  Current path: %s (exists: %d)\n", curr_path, has_curr);
                                        #endif

                                        /* Only add sensors with both voltage and current */
                                        if (has_volt && has_curr)
                                        {
                                                /* Allocate memory for sensor info */
                                                int idx = handle->sensor_count;

                                                /* Resize the sensor arrays */
                                                handle->sensor_count++;
                                                handle->sensor_names = realloc(handle->sensor_names,
                                                                               handle->sensor_count * sizeof(char *));
                                                handle->sensor_paths = realloc(handle->sensor_paths,
                                                                               handle->sensor_count * sizeof(char *));
                                                handle->sensor_types = realloc(handle->sensor_types,
                                                                               handle->sensor_count * sizeof(pm_sensor_type_t));

                                                if (!handle->sensor_names || !handle->sensor_paths || !handle->sensor_types)
                                                {
                                                        fclose(fp);
                                                        return PM_ERROR_MEMORY;
                                                }

                                                /* Store sensor information */
                                                handle->sensor_names[idx] = strdup_safe(buffer);
                                                handle->sensor_paths[idx] = strdup_safe(path);
                                                handle->sensor_types[idx] = PM_SENSOR_TYPE_I2C;

                                                printf("Found I2C power sensor: %s (port %d)\n", buffer, port_number);
                                        }
                                        #ifdef SHOW_ALL_DEBUG
                                        else
                                        {
                                                printf("Skipped sensor %s: missing voltage or current capability\n", buffer);
                                        }
                                        #endif
                                }
                                fclose(fp);
                        }
                }
        }

        closedir(dir);
        return PM_SUCCESS;
}

/* Read sensor data */
static pm_error_t read_sensor_data(pm_handle_t handle)
{
        /* Lock the mutex to update the data */
        pthread_mutex_lock(&handle->data_mutex);

        /* Update the time */
        clock_gettime(CLOCK_REALTIME, &handle->last_sample_time);

        /* Read data from each sensor */
        for (int i = 0; i < handle->sensor_count; i++)
        {
                char volt_path[512], curr_path[512];
                FILE *fp;
                double voltage = 0.0, current = 0.0;
                bool read_success = true;
                int port_number = -1;

                /* Get port number from sensor name */
                if (handle->sensor_types[i] == PM_SENSOR_TYPE_I2C)
                {
                        char *name = handle->sensor_names[i];
                        if (strstr(name, "VDD_IN")) port_number = 1;
                        else if (strstr(name, "VDD_CPU_GPU_CV")) port_number = 2;
                        else if (strstr(name, "VDD_SOC")) port_number = 3;
                        else if (strstr(name, "VDD_DDR")) port_number = 4;
                        else if (strstr(name, "VDD_5V0")) port_number = 5;
                        else if (strstr(name, "VDD_3V3")) port_number = 6;
                        else if (strstr(name, "VDD_1V8")) port_number = 7;
                }

                /* Construct paths based on sensor type */
                if (handle->sensor_types[i] == PM_SENSOR_TYPE_I2C)
                {
                        /* Try both hwmon and iio formats */
                        if (strstr(handle->sensor_paths[i], "hwmon"))
                        {
                                /* Try hwmon format first */
                                snprintf(volt_path, sizeof(volt_path), "%s/in%d_input", 
                                        handle->sensor_paths[i], port_number);
                                snprintf(curr_path, sizeof(curr_path), "%s/curr%d_input", 
                                        handle->sensor_paths[i], port_number);

                                /* If files don't exist, try alternative hwmon format */
                                if (!check_file_exists(volt_path) || !check_file_exists(curr_path))
                                {
                                        snprintf(volt_path, sizeof(volt_path), "%s/voltage%d_input", 
                                                handle->sensor_paths[i], port_number);
                                        snprintf(curr_path, sizeof(curr_path), "%s/current%d_input", 
                                                handle->sensor_paths[i], port_number);
                                }
                        }
                        else
                        {
                                /* Try iio format */
                                snprintf(volt_path, sizeof(volt_path), "%s/in_voltage%d_input", 
                                        handle->sensor_paths[i], port_number);
                                snprintf(curr_path, sizeof(curr_path), "%s/in_current%d_input", 
                                        handle->sensor_paths[i], port_number);
                        }

                        #ifdef SHOW_ALL_DEBUG
                        printf("Reading sensor %s (port %d):\n", handle->sensor_names[i], port_number);
                        printf("  Voltage path: %s\n", volt_path);
                        printf("  Current path: %s\n", curr_path);
                        #endif
                }
                else /* PM_SENSOR_TYPE_SYSTEM */
                {
                        snprintf(volt_path, sizeof(volt_path), "%s/voltage_now", 
                                handle->sensor_paths[i]);
                        snprintf(curr_path, sizeof(curr_path), "%s/current_now", 
                                handle->sensor_paths[i]);
                }

                /* Read voltage */
                fp = fopen(volt_path, "r");
                if (fp)
                {
                        char line[256];
                        if (fgets(line, sizeof(line), fp))
                        {
                                voltage = strtod(line, NULL);
                                #ifdef SHOW_ALL_DEBUG
                                printf("  Raw voltage: %lf\n", voltage);
                                #endif
                        }
                        else
                        {
                                read_success = false;
                                #ifdef SHOW_ALL_DEBUG
                                printf("  Failed to read voltage from %s\n", volt_path);
                                #endif
                        }
                        fclose(fp);
                }
                else
                {
                        read_success = false;
                        #ifdef SHOW_ALL_DEBUG
                        printf("  Cannot open voltage file: %s\n", volt_path);
                        #endif
                }

                /* Read current */
                fp = fopen(curr_path, "r");
                if (fp)
                {
                        char line[256];
                        if (fgets(line, sizeof(line), fp))
                        {
                                current = strtod(line, NULL);
                                #ifdef SHOW_ALL_DEBUG
                                printf("  Raw current: %lf\n", current);
                                #endif
                        }
                        else
                        {
                                read_success = false;
                                #ifdef SHOW_ALL_DEBUG
                                printf("  Failed to read current from %s\n", curr_path);
                                #endif
                        }
                        fclose(fp);
                }
                else
                {
                        read_success = false;
                        #ifdef SHOW_ALL_DEBUG
                        printf("  Cannot open current file: %s\n", curr_path);
                        #endif
                }

                /* Convert units and validate readings */
                if (read_success)
                {
                        /* Convert from mV to V and mA to A */
                        voltage /= 1000.0;
                        current /= 1000.0;

                        /* Validate readings based on sensor type */
                        bool valid_reading = true;
                        if (strstr(handle->sensor_names[i], "VDD_IN"))
                        {
                                if (voltage < 10.0 || voltage > 25.0) valid_reading = false;
                                if (current < 0.0 || current > 10.0) valid_reading = false;
                        }
                        else if (strstr(handle->sensor_names[i], "VDD_CPU_GPU_CV"))
                        {
                                if (voltage < 0.5 || voltage > 1.5) valid_reading = false;
                                if (current < 0.0 || current > 20.0) valid_reading = false;
                        }
                        else if (strstr(handle->sensor_names[i], "VDD_SOC"))
                        {
                                if (voltage < 0.5 || voltage > 1.5) valid_reading = false;
                                if (current < 0.0 || current > 10.0) valid_reading = false;
                        }
                        else if (strstr(handle->sensor_names[i], "VDD_DDR"))
                        {
                                if (voltage < 1.0 || voltage > 2.0) valid_reading = false;
                                if (current < 0.0 || current > 5.0) valid_reading = false;
                        }
                        else if (strstr(handle->sensor_names[i], "VDD_5V0"))
                        {
                                if (voltage < 4.5 || voltage > 5.5) valid_reading = false;
                                if (current < 0.0 || current > 2.0) valid_reading = false;
                        }
                        else if (strstr(handle->sensor_names[i], "VDD_3V3"))
                        {
                                if (voltage < 3.0 || voltage > 3.6) valid_reading = false;
                                if (current < 0.0 || current > 2.0) valid_reading = false;
                        }
                        else if (strstr(handle->sensor_names[i], "VDD_1V8"))
                        {
                                if (voltage < 1.6 || voltage > 2.0) valid_reading = false;
                                if (current < 0.0 || current > 2.0) valid_reading = false;
                        }

                        if (!valid_reading)
                        {
                                #ifdef SHOW_ALL_DEBUG
                                printf("  Invalid reading for %s: %.3f V, %.3f A\n", 
                                       handle->sensor_names[i], voltage, current);
                                #endif
                                read_success = false;
                        }

                        #ifdef SHOW_ALL_DEBUG
                        if (read_success)
                        {
                                printf("  Converted values: %.3f V, %.3f A\n", voltage, current);
                                printf("  Calculated power: %.3f W\n", voltage * current);
                        }
                        #endif
                }

                /* Update sensor data */
                handle->latest_data.sensors[i].voltage = voltage;
                handle->latest_data.sensors[i].current = current;
                handle->latest_data.sensors[i].power = voltage * current;
                handle->latest_data.sensors[i].online = read_success;
                strncpy(handle->latest_data.sensors[i].status, 
                        read_success ? "Normal" : "Error", 
                        sizeof(handle->latest_data.sensors[i].status) - 1);

                /* Set thresholds based on sensor type */
                if (strstr(handle->sensor_names[i], "VDD_IN"))
                {
                        handle->latest_data.sensors[i].warning_threshold = 15.0;
                        handle->latest_data.sensors[i].critical_threshold = 20.0;
                }
                else if (strstr(handle->sensor_names[i], "VDD_CPU_GPU_CV"))
                {
                        handle->latest_data.sensors[i].warning_threshold = 10.0;
                        handle->latest_data.sensors[i].critical_threshold = 15.0;
                }
                else if (strstr(handle->sensor_names[i], "VDD_SOC"))
                {
                        handle->latest_data.sensors[i].warning_threshold = 5.0;
                        handle->latest_data.sensors[i].critical_threshold = 8.0;
                }
                else
                {
                        handle->latest_data.sensors[i].warning_threshold = 3.0;
                        handle->latest_data.sensors[i].critical_threshold = 5.0;
                }
        }

        /* Calculate total power */
        calculate_total_power(handle);

        pthread_mutex_unlock(&handle->data_mutex);
        return PM_SUCCESS;
}

/* Calculate the total power from all sensors */
static void calculate_total_power(pm_handle_t handle)
{
        /* Find VDD_IN sensor */
        for (int i = 0; i < handle->sensor_count; i++)
        {
                if (strstr(handle->sensor_names[i], "VDD_IN"))
                {
                        /* Use VDD_IN as total power */
                        strncpy(handle->latest_data.total.name, "Total (VDD_IN)", 
                                sizeof(handle->latest_data.total.name) - 1);
                        handle->latest_data.total.power = handle->latest_data.sensors[i].power;
                        handle->latest_data.total.current = handle->latest_data.sensors[i].current;
                        handle->latest_data.total.voltage = handle->latest_data.sensors[i].voltage;
                        handle->latest_data.total.online = handle->latest_data.sensors[i].online;
                        strncpy(handle->latest_data.total.status, handle->latest_data.sensors[i].status,
                                sizeof(handle->latest_data.total.status) - 1);
                        handle->latest_data.total.warning_threshold = 25.0;
                        handle->latest_data.total.critical_threshold = 35.0;
                        return;
                }
        }

        /* If VDD_IN not found, use sum of all sensors (fallback) */
        double total_power = 0.0;
        double total_current = 0.0;
        double total_voltage = 0.0;
        bool all_online = true;

        for (int i = 0; i < handle->sensor_count; i++)
        {
                total_power += handle->latest_data.sensors[i].power;
                total_current += handle->latest_data.sensors[i].current;
                if (handle->latest_data.sensors[i].voltage > total_voltage)
                        total_voltage = handle->latest_data.sensors[i].voltage;
                if (!handle->latest_data.sensors[i].online)
                        all_online = false;
        }

        /* Update the total values */
        strncpy(handle->latest_data.total.name, "Total (Sum)", 
                sizeof(handle->latest_data.total.name) - 1);
        handle->latest_data.total.power = total_power;
        handle->latest_data.total.current = total_current;
        handle->latest_data.total.voltage = total_voltage;
        handle->latest_data.total.online = all_online;
        strncpy(handle->latest_data.total.status, all_online ? "Normal" : "Partial",
                sizeof(handle->latest_data.total.status) - 1);
        handle->latest_data.total.warning_threshold = 25.0;
        handle->latest_data.total.critical_threshold = 35.0;
}

/* Update the statistics */
static pm_error_t update_statistics(pm_handle_t handle)
{
        /* Lock the mutex to update the statistics */
        pthread_mutex_lock(&handle->data_mutex);

        /* Update the sensor statistics */
        for (int i = 0; i < handle->sensor_count; i++)
        {
                /* Voltage */
                if (handle->statistics.sensors[i].voltage.count == 0)
                {
                        handle->statistics.sensors[i].voltage.min = handle->latest_data.sensors[i].voltage;
                        handle->statistics.sensors[i].voltage.max = handle->latest_data.sensors[i].voltage;
                }
                else
                {
                        if (handle->latest_data.sensors[i].voltage < handle->statistics.sensors[i].voltage.min)
                        {
                                handle->statistics.sensors[i].voltage.min = handle->latest_data.sensors[i].voltage;
                        }
                        if (handle->latest_data.sensors[i].voltage > handle->statistics.sensors[i].voltage.max)
                        {
                                handle->statistics.sensors[i].voltage.max = handle->latest_data.sensors[i].voltage;
                        }
                }

                handle->statistics.sensors[i].voltage.total += handle->latest_data.sensors[i].voltage;
                handle->statistics.sensors[i].voltage.count++;
                handle->statistics.sensors[i].voltage.avg = handle->statistics.sensors[i].voltage.total / handle->statistics.sensors[i].voltage.count;

                /* Current */
                if (handle->statistics.sensors[i].current.count == 0)
                {
                        handle->statistics.sensors[i].current.min = handle->latest_data.sensors[i].current;
                        handle->statistics.sensors[i].current.max = handle->latest_data.sensors[i].current;
                }
                else
                {
                        if (handle->latest_data.sensors[i].current < handle->statistics.sensors[i].current.min)
                        {
                                handle->statistics.sensors[i].current.min = handle->latest_data.sensors[i].current;
                        }
                        if (handle->latest_data.sensors[i].current > handle->statistics.sensors[i].current.max)
                        {
                                handle->statistics.sensors[i].current.max = handle->latest_data.sensors[i].current;
                        }
                }

                handle->statistics.sensors[i].current.total += handle->latest_data.sensors[i].current;
                handle->statistics.sensors[i].current.count++;
                handle->statistics.sensors[i].current.avg = handle->statistics.sensors[i].current.total / handle->statistics.sensors[i].current.count;

                /* Power */
                if (handle->statistics.sensors[i].power.count == 0)
                {
                        handle->statistics.sensors[i].power.min = handle->latest_data.sensors[i].power;
                        handle->statistics.sensors[i].power.max = handle->latest_data.sensors[i].power;
                }
                else
                {
                        if (handle->latest_data.sensors[i].power < handle->statistics.sensors[i].power.min)
                        {
                                handle->statistics.sensors[i].power.min = handle->latest_data.sensors[i].power;
                        }
                        if (handle->latest_data.sensors[i].power > handle->statistics.sensors[i].power.max)
                        {
                                handle->statistics.sensors[i].power.max = handle->latest_data.sensors[i].power;
                        }
                }

                handle->statistics.sensors[i].power.total += handle->latest_data.sensors[i].power;
                handle->statistics.sensors[i].power.count++;
                handle->statistics.sensors[i].power.avg = handle->statistics.sensors[i].power.total / handle->statistics.sensors[i].power.count;
        }

        /* Update the total statistics */
        if (handle->statistics.total.power.count == 0)
        {
                handle->statistics.total.power.min = handle->latest_data.total.power;
                handle->statistics.total.power.max = handle->latest_data.total.power;
                handle->statistics.total.current.min = handle->latest_data.total.current;
                handle->statistics.total.current.max = handle->latest_data.total.current;
                handle->statistics.total.voltage.min = handle->latest_data.total.voltage;
                handle->statistics.total.voltage.max = handle->latest_data.total.voltage;
        }
        else
        {
                /* Power */
                if (handle->latest_data.total.power < handle->statistics.total.power.min)
                {
                        handle->statistics.total.power.min = handle->latest_data.total.power;
                }
                if (handle->latest_data.total.power > handle->statistics.total.power.max)
                {
                        handle->statistics.total.power.max = handle->latest_data.total.power;
                }

                /* Current */
                if (handle->latest_data.total.current < handle->statistics.total.current.min)
                {
                        handle->statistics.total.current.min = handle->latest_data.total.current;
                }
                if (handle->latest_data.total.current > handle->statistics.total.current.max)
                {
                        handle->statistics.total.current.max = handle->latest_data.total.current;
                }

                /* Voltage */
                if (handle->latest_data.total.voltage < handle->statistics.total.voltage.min)
                {
                        handle->statistics.total.voltage.min = handle->latest_data.total.voltage;
                }
                if (handle->latest_data.total.voltage > handle->statistics.total.voltage.max)
                {
                        handle->statistics.total.voltage.max = handle->latest_data.total.voltage;
                }
        }

        handle->statistics.total.power.total += handle->latest_data.total.power;
        handle->statistics.total.power.count++;
        handle->statistics.total.power.avg = handle->statistics.total.power.total / handle->statistics.total.power.count;

        handle->statistics.total.current.total += handle->latest_data.total.current;
        handle->statistics.total.current.count++;
        handle->statistics.total.current.avg = handle->statistics.total.current.total / handle->statistics.total.current.count;

        handle->statistics.total.voltage.total += handle->latest_data.total.voltage;
        handle->statistics.total.voltage.count++;
        handle->statistics.total.voltage.avg = handle->statistics.total.voltage.total / handle->statistics.total.voltage.count;

        pthread_mutex_unlock(&handle->data_mutex);
        return PM_SUCCESS;
}

/* Check if a file exists */
static bool check_file_exists(const char *path)
{
        return access(path, F_OK) == 0;
}

/* Safe version of strdup that checks for NULL */
static char *strdup_safe(const char *str)
{
        if (!str)
        {
                return NULL;
        }

        char *result = strdup(str);
        if (!result)
        {
                /* Handle memory allocation failure */
                /* In a real implementation, this would log an error */
        }

        return result;
}