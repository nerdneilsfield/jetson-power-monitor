/**
 * @file jetpwmon++.hpp
 * @brief C++ RAII wrapper for jetpwmon library
 * @author Qi Deng<dengqi935@gmail.com>
 */

#pragma once

#include <jetpwmon/jetpwmon.h>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace jetpwmon {

/**
 * @brief RAII wrapper for pm_power_stats_t
 */
class PowerStats {
public:
    PowerStats(const pm_power_stats_t& stats);
    ~PowerStats();

    // Delete copy constructor and assignment operator
    PowerStats(const PowerStats&) = delete;
    PowerStats& operator=(const PowerStats&) = delete;

    // Allow move
    PowerStats(PowerStats&&) noexcept;
    PowerStats& operator=(PowerStats&&) noexcept;

    // Getters
    const pm_sensor_stats_t& getTotal() const { return total_; }
    const pm_sensor_stats_t* getSensors() const { return sensors_; }
    int getSensorCount() const { return sensor_count_; }

private:
    pm_sensor_stats_t total_;
    pm_sensor_stats_t* sensors_;
    int sensor_count_;
};

/**
 * @brief RAII wrapper for pm_power_data_t
 */
class PowerData {
public:
    PowerData(const pm_power_data_t& data);
    ~PowerData();

    // Delete copy constructor and assignment operator
    PowerData(const PowerData&) = delete;
    PowerData& operator=(const PowerData&) = delete;

    // Allow move
    PowerData(PowerData&&) noexcept;
    PowerData& operator=(PowerData&&) noexcept;

    // Getters
    const pm_sensor_data_t& getTotal() const { return total_; }
    const pm_sensor_data_t* getSensors() const { return sensors_; }
    int getSensorCount() const { return sensor_count_; }

private:
    pm_sensor_data_t total_;
    pm_sensor_data_t* sensors_;
    int sensor_count_;
};

/**
 * @brief RAII wrapper for the power monitoring library
 */
class PowerMonitor {
public:
    /**
     * @brief Constructor that initializes the power monitor
     * @throw std::runtime_error if initialization fails
     */
    PowerMonitor();

    /**
     * @brief Destructor that cleans up resources
     */
    ~PowerMonitor();

    /**
     * @brief Set sampling frequency
     * @param frequency_hz Sampling frequency in Hz
     * @throw std::runtime_error if setting frequency fails
     */
    void setSamplingFrequency(int frequency_hz);

    /**
     * @brief Get current sampling frequency
     * @return Sampling frequency in Hz
     * @throw std::runtime_error if getting frequency fails
     */
    int getSamplingFrequency() const;

    /**
     * @brief Start sampling
     * @throw std::runtime_error if starting sampling fails
     */
    void startSampling();

    /**
     * @brief Stop sampling
     * @throw std::runtime_error if stopping sampling fails
     */
    void stopSampling();

    /**
     * @brief Check if sampling is active
     * @return true if sampling is active, false otherwise
     * @throw std::runtime_error if checking status fails
     */
    bool isSampling() const;

    /**
     * @brief Get latest power data
     * @return Power data structure
     * @throw std::runtime_error if getting data fails
     */
    PowerData getLatestData() const;

    /**
     * @brief Get power statistics
     * @return Power statistics structure
     * @throw std::runtime_error if getting statistics fails
     */
    PowerStats getStatistics() const;

    /**
     * @brief Reset statistics
     * @throw std::runtime_error if resetting statistics fails
     */
    void resetStatistics();

    /**
     * @brief Get number of sensors
     * @return Number of sensors
     * @throw std::runtime_error if getting sensor count fails
     */
    int getSensorCount() const;

    /**
     * @brief Get sensor names
     * @return Vector of sensor names
     * @throw std::runtime_error if getting sensor names fails
     */
    std::vector<std::string> getSensorNames() const;

    // Delete copy constructor and assignment operator
    PowerMonitor(const PowerMonitor&) = delete;
    PowerMonitor& operator=(const PowerMonitor&) = delete;

    // Allow move
    PowerMonitor(PowerMonitor&&) noexcept;
    PowerMonitor& operator=(PowerMonitor&&) noexcept;

private:
    struct HandleDeleter {
        void operator()(pm_handle_t* handle) const {
            if (handle) pm_cleanup(*handle);
        }
    };
    std::unique_ptr<pm_handle_t, HandleDeleter> handle_;
};

} // namespace jetpwmon
