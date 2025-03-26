/**
 * @file jetpwmon++.cpp
 * @brief Implementation of the C++ RAII wrapper for jetpwmon
 */

#include <jetpwmon/jetpwmon++.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

namespace jetpwmon {

PowerMonitor::PowerMonitor() : handle_(nullptr, pm_cleanup) {
    pm_handle_t handle;
    pm_error_t error = pm_init(&handle);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    handle_.reset(handle);
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
    return data;
}

pm_power_stats_t PowerMonitor::getStatistics() const {
    pm_power_stats_t stats;
    pm_error_t error = pm_get_statistics(handle_.get(), &stats);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    return stats;
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
