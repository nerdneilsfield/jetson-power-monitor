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

// PowerStats implementation
PowerStats::PowerStats(const pm_power_stats_t& stats) 
    : sensor_count_(stats.sensor_count) {
    // Copy total stats
    std::memcpy(&total_, &stats.total, sizeof(pm_sensor_stats_t));
    
    // Allocate and copy sensor data
    if (sensor_count_ > 0) {
        sensors_ = new pm_sensor_stats_t[sensor_count_];
        for (int i = 0; i < sensor_count_; i++) {
            std::memcpy(&sensors_[i], &stats.sensors[i], sizeof(pm_sensor_stats_t));
        }
    } else {
        sensors_ = nullptr;
    }
}

PowerStats::~PowerStats() {
    if (sensors_) {
        delete[] sensors_;
    }
}

PowerStats::PowerStats(PowerStats&& other) noexcept
    : total_(other.total_)
    , sensors_(other.sensors_)
    , sensor_count_(other.sensor_count_) {
    other.sensors_ = nullptr;
}

PowerStats& PowerStats::operator=(PowerStats&& other) noexcept {
    if (this != &other) {
        if (sensors_) {
            delete[] sensors_;
        }
        total_ = other.total_;
        sensors_ = other.sensors_;
        sensor_count_ = other.sensor_count_;
        other.sensors_ = nullptr;
    }
    return *this;
}

// PowerData implementation
PowerData::PowerData(const pm_power_data_t& data)
    : sensor_count_(data.sensor_count) {
    // Copy total data
    std::memcpy(&total_, &data.total, sizeof(pm_sensor_data_t));
    
    // Allocate and copy sensor data
    if (sensor_count_ > 0) {
        sensors_ = new pm_sensor_data_t[sensor_count_];
        for (int i = 0; i < sensor_count_; i++) {
            std::memcpy(&sensors_[i], &data.sensors[i], sizeof(pm_sensor_data_t));
        }
    } else {
        sensors_ = nullptr;
    }
}

PowerData::~PowerData() {
    if (sensors_) {
        delete[] sensors_;
    }
}

PowerData::PowerData(PowerData&& other) noexcept
    : total_(other.total_)
    , sensors_(other.sensors_)
    , sensor_count_(other.sensor_count_) {
    other.sensors_ = nullptr;
}

PowerData& PowerData::operator=(PowerData&& other) noexcept {
    if (this != &other) {
        if (sensors_) {
            delete[] sensors_;
        }
        total_ = other.total_;
        sensors_ = other.sensors_;
        sensor_count_ = other.sensor_count_;
        other.sensors_ = nullptr;
    }
    return *this;
}

// PowerMonitor implementation
PowerMonitor::PowerMonitor() : handle_(nullptr) {
    pm_handle_t handle;
    pm_error_t error = pm_init(&handle);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    handle_.reset(new pm_handle_t(handle));
}

PowerMonitor::~PowerMonitor() = default;

PowerMonitor::PowerMonitor(PowerMonitor&& other) noexcept
    : handle_(std::move(other.handle_)) {}

PowerMonitor& PowerMonitor::operator=(PowerMonitor&& other) noexcept {
    if (this != &other) {
        handle_ = std::move(other.handle_);
    }
    return *this;
}

void PowerMonitor::setSamplingFrequency(int frequency_hz) {
    pm_error_t error = pm_set_sampling_frequency(*handle_.get(), frequency_hz);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
}

int PowerMonitor::getSamplingFrequency() const {
    int frequency_hz;
    pm_error_t error = pm_get_sampling_frequency(*handle_.get(), &frequency_hz);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    return frequency_hz;
}

void PowerMonitor::startSampling() {
    pm_error_t error = pm_start_sampling(*handle_.get());
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
}

void PowerMonitor::stopSampling() {
    pm_error_t error = pm_stop_sampling(*handle_.get());
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
}

bool PowerMonitor::isSampling() const {
    bool is_sampling;
    pm_error_t error = pm_is_sampling(*handle_.get(), &is_sampling);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    return is_sampling;
}

PowerData PowerMonitor::getLatestData() const {
    pm_power_data_t data;
    pm_error_t error = pm_get_latest_data(*handle_.get(), &data);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    return PowerData(data);
}

PowerStats PowerMonitor::getStatistics() const {
    pm_power_stats_t stats;
    pm_error_t error = pm_get_statistics(*handle_.get(), &stats);
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
    return PowerStats(stats);
}

void PowerMonitor::resetStatistics() {
    pm_error_t error = pm_reset_statistics(*handle_.get());
    if (error != PM_SUCCESS) {
        throw std::runtime_error(pm_error_string(error));
    }
}

int PowerMonitor::getSensorCount() const {
    int count;
    pm_error_t error = pm_get_sensor_count(*handle_.get(), &count);
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

    pm_error_t error = pm_get_sensor_names(*handle_.get(), names.data(), &count);
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
