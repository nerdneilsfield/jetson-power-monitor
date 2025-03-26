/**
 * @file jetpwmon.h
 * @brief A library for power consumption monitoring and statistics
 * @author Qi Deng<dengqi935@gmail.com>
 *
 * This library provides functionality to monitor power consumption
 * from various sources (I2C sensors, system power supplies), collect
 * statistics, and control the sampling process.
 */

#ifndef JETPWMON_H
#define JETPWMON_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes returned by library functions
 */
typedef enum {
    PM_SUCCESS = 0,                  /**< Operation completed successfully */
    PM_ERROR_INIT_FAILED = -1,       /**< Initialization failed */
    PM_ERROR_NOT_INITIALIZED = -2,   /**< Library not initialized */
    PM_ERROR_ALREADY_RUNNING = -3,   /**< Sampling already running */
    PM_ERROR_NOT_RUNNING = -4,       /**< Sampling not running */
    PM_ERROR_INVALID_FREQUENCY = -5, /**< Invalid sampling frequency */
    PM_ERROR_NO_SENSORS = -6,        /**< No power sensors found */
    PM_ERROR_FILE_ACCESS = -7,       /**< Error accessing sensor files */
    PM_ERROR_MEMORY = -8,            /**< Memory allocation error */
    PM_ERROR_THREAD = -9             /**< Thread creation/management error */
} pm_error_t;

/**
 * @brief Sensor types
 */
typedef enum {
    PM_SENSOR_TYPE_UNKNOWN = 0,      /**< Unknown sensor type */
    PM_SENSOR_TYPE_I2C = 1,          /**< I2C power sensor (e.g., INA3221) */
    PM_SENSOR_TYPE_SYSTEM = 2        /**< System power supply */
} pm_sensor_type_t;

/**
 * @brief Power data for a single sensor
 */
typedef struct {
    char name[64];                   /**< Sensor name */
    pm_sensor_type_t type;           /**< Sensor type */
    double voltage;                  /**< Voltage in volts */
    double current;                  /**< Current in amperes */
    double power;                    /**< Power in watts */
    bool online;                     /**< Whether the sensor is online */
    char status[32];                 /**< Status string (if available) */
    double warning_threshold;        /**< Warning threshold in watts */
    double critical_threshold;       /**< Critical threshold in watts */
} pm_sensor_data_t;

/**
 * @brief Statistical data
 */
typedef struct {
    double min;                      /**< Minimum value */
    double max;                      /**< Maximum value */
    double avg;                      /**< Average value */
    double total;                    /**< Sum of all samples */
    uint64_t count;                  /**< Number of samples */
} pm_stats_t;

/**
 * @brief Power statistics for a sensor
 */
typedef struct {
    char name[64];                   /**< Sensor name */
    pm_stats_t voltage;              /**< Voltage statistics */
    pm_stats_t current;              /**< Current statistics */
    pm_stats_t power;                /**< Power statistics */
} pm_sensor_stats_t;

/**
 * @brief Overall power data
 */
typedef struct {
    pm_sensor_data_t total;          /**< Total power consumption */
    pm_sensor_data_t* sensors;       /**< Array of sensor data */
    int sensor_count;                /**< Number of sensors */
} pm_power_data_t;

/**
 * @brief Overall power statistics
 */
typedef struct {
    pm_sensor_stats_t total;         /**< Total power statistics */
    pm_sensor_stats_t* sensors;      /**< Array of sensor statistics */
    int sensor_count;                /**< Number of sensors */
} pm_power_stats_t;

/**
 * @brief Library handle
 */
typedef struct pm_handle_s* pm_handle_t;

/**
 * @brief Initialize the power monitor
 *
 * This function discovers power sensors on the system and initializes
 * the power monitor library.
 *
 * @param[out] handle Pointer to store the library handle
 * @return Error code
 */
pm_error_t pm_init(pm_handle_t* handle);

/**
 * @brief Clean up resources
 *
 * This function stops any active sampling and frees all resources
 * allocated by the library.
 *
 * @param handle Library handle
 * @return Error code
 */
pm_error_t pm_cleanup(pm_handle_t handle);

/**
 * @brief Set the sampling frequency
 *
 * @param handle Library handle
 * @param frequency_hz Sampling frequency in Hz (must be > 0)
 * @return Error code
 */
pm_error_t pm_set_sampling_frequency(pm_handle_t handle, int frequency_hz);

/**
 * @brief Get the current sampling frequency
 *
 * @param handle Library handle
 * @param[out] frequency_hz Pointer to store the frequency
 * @return Error code
 */
pm_error_t pm_get_sampling_frequency(pm_handle_t handle, int* frequency_hz);

/**
 * @brief Start sampling
 *
 * This function starts the sampling thread that periodically reads
 * power data from all discovered sensors.
 *
 * @param handle Library handle
 * @return Error code
 */
pm_error_t pm_start_sampling(pm_handle_t handle);

/**
 * @brief Stop sampling
 *
 * This function stops the sampling thread.
 *
 * @param handle Library handle
 * @return Error code
 */
pm_error_t pm_stop_sampling(pm_handle_t handle);

/**
 * @brief Check if sampling is active
 *
 * @param handle Library handle
 * @param[out] is_sampling Pointer to store the result
 * @return Error code
 */
pm_error_t pm_is_sampling(pm_handle_t handle, bool* is_sampling);

/**
 * @brief Get the latest power data
 *
 * @param handle Library handle
 * @param[out] data Pointer to store the data
 * @return Error code
 */
pm_error_t pm_get_latest_data(pm_handle_t handle, pm_power_data_t* data);

/**
 * @brief Get the power statistics
 *
 * @param handle Library handle
 * @param[out] stats Pointer to store the statistics
 * @return Error code
 */
pm_error_t pm_get_statistics(pm_handle_t handle, pm_power_stats_t* stats);

/**
 * @brief Reset the statistics
 *
 * This function resets all collected statistics.
 *
 * @param handle Library handle
 * @return Error code
 */
pm_error_t pm_reset_statistics(pm_handle_t handle);

/**
 * @brief Get the number of sensors
 *
 * @param handle Library handle
 * @param[out] count Pointer to store the count
 * @return Error code
 */
pm_error_t pm_get_sensor_count(pm_handle_t handle, int* count);

/**
 * @brief Get the sensor names
 *
 * This function fills an array of strings with the names of all sensors.
 * The array must be pre-allocated with enough space for all sensor names.
 *
 * @param handle Library handle
 * @param[out] names Array to store the names
 * @param[inout] count On input: size of the array; On output: number of sensors
 * @return Error code
 */
pm_error_t pm_get_sensor_names(pm_handle_t handle, char** names, int* count);

/**
 * @brief Get a human-readable error message for an error code
 *
 * @param error Error code
 * @return Error message
 */
const char* pm_error_string(pm_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* JETPWMON_H */