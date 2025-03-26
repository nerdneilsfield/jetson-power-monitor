/**
 * @file jetpwmon++.cpp
 * @brief Implementation of the C++ RAII wrapper for jetpwmon
 */

#include <jetpwmon/jetpwmon++.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <cstring>

namespace jetpwmon {

PowerMonitor::PowerMonitor() : handle_(nullptr, pm_cleanup) {
    pm_handle_t handle;
    pm_error_t error = pm_init(&handle);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    handle_.reset(handle);
}

PowerMonitor::~PowerMonitor() {
    // unique_ptr will automatically call pm_cleanup
}

void PowerMonitor::setSamplingFrequency(int frequency_hz) {
    pm_error_t error = pm_set_sampling_frequency(handle_.get(), frequency_hz);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
}

int PowerMonitor::getSamplingFrequency() const {
    int frequency_hz;
    pm_error_t error = pm_get_sampling_frequency(handle_.get(), &frequency_hz);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    return frequency_hz;
}

void PowerMonitor::startSampling() {
    pm_error_t error = pm_start_sampling(handle_.get());
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
}

void PowerMonitor::stopSampling() {
    pm_error_t error = pm_stop_sampling(handle_.get());
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
}

bool PowerMonitor::isSampling() const {
    bool is_sampling;
    pm_error_t error = pm_is_sampling(handle_.get(), &is_sampling);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    return is_sampling;
}

pm_power_data_t PowerMonitor::getLatestData() const {
    pm_power_data_t data;
    pm_error_t error = pm_get_latest_data(handle_.get(), &data);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }

    // Create a complete copy
    pm_power_data_t result;
    result.sensor_count = data.sensor_count;
    
    // Copy total power data
    std::memcpy(&result.total, &data.total, sizeof(pm_sensor_data_t));
    
    // Allocate and copy sensor data
    if (data.sensor_count > 0) {
        result.sensors = new pm_sensor_data_t[data.sensor_count];
        for (int i = 0; i < data.sensor_count; i++) {
            std::memcpy(&result.sensors[i], &data.sensors[i], sizeof(pm_sensor_data_t));
        }
    } else {
        result.sensors = nullptr;
    }

    return result;
}

pm_power_stats_t PowerMonitor::getStatistics() const {
    pm_power_stats_t stats;
    pm_error_t error = pm_get_statistics(handle_.get(), &stats);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }

    // Create a complete copy
    pm_power_stats_t result;
    result.sensor_count = stats.sensor_count;
    
    // Copy total power statistics
    std::memcpy(&result.total, &stats.total, sizeof(pm_stats_t));
    
    // Copy sensor name
    std::strncpy(result.total.name, stats.total.name, sizeof(result.total.name) - 1);
    result.total.name[sizeof(result.total.name) - 1] = '\0';
    
    // Allocate and copy sensor data
    if (stats.sensor_count > 0) {
        result.sensors = new pm_sensor_stats_t[stats.sensor_count];
        for (int i = 0; i < stats.sensor_count; i++) {
            // Copy sensor name
            std::strncpy(result.sensors[i].name, stats.sensors[i].name, sizeof(result.sensors[i].name) - 1);
            result.sensors[i].name[sizeof(result.sensors[i].name) - 1] = '\0';
            
            // Copy statistics
            std::memcpy(&result.sensors[i].voltage, &stats.sensors[i].voltage, sizeof(pm_stats_t));
            std::memcpy(&result.sensors[i].current, &stats.sensors[i].current, sizeof(pm_stats_t));
            std::memcpy(&result.sensors[i].power, &stats.sensors[i].power, sizeof(pm_stats_t));
        }
    } else {
        result.sensors = nullptr;
    }

    return result;
}

void PowerMonitor::resetStatistics() {
    pm_error_t error = pm_reset_statistics(handle_.get());
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
}

int PowerMonitor::getSensorCount() const {
    int count;
    pm_error_t error = pm_get_sensor_count(handle_.get(), &count);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    return count;
}

std::vector<std::string> PowerMonitor::getSensorNames() const {
    int count = getSensorCount();
    std::vector<char*> names(count);
    for (int i = 0; i < count; ++i) {
        names[i] = new char[64];
    }

    pm_error_t error = pm_get_sensor_names(handle_.get(), names.data(), &count);
    if (error != PM_SUCCESS) {
        for (int i = 0; i < count; ++i) {
            delete[] names[i];
        }
        throw std::runtime_error(pm_error_string(error));
    }

    std::vector<std::string> result;
    for (int i = 0; i < count; ++i) {
        result.push_back(names[i]);
        delete[] names[i];
    }
    return result;
}

} // namespace jetpwmon
