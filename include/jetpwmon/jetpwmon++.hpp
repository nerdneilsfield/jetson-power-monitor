/**
 * @file jetpwmon++.hpp
 * @brief C++ RAII wrapper for jetpwmon library
 * @author Qi Deng<dengqi935@gmail.com>
 */

#ifndef JETPWMON_PLUS_PLUS_HPP
#define JETPWMON_PLUS_PLUS_HPP

#include <jetpwmon/jetpwmon.h>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace jetpwmon {

class PowerMonitor {
public:
    /**
     * @brief Constructor that initializes the power monitor
     * @throw std::runtime_error if initialization fails
     */
    PowerMonitor();

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
    pm_power_data_t getLatestData() const;

    /**
     * @brief Get power statistics
     * @return Power statistics structure
     * @throw std::runtime_error if getting statistics fails
     */
    pm_power_stats_t getStatistics() const;

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

private:
    std::unique_ptr<pm_handle_s, decltype(&pm_cleanup)> handle_;
};

} // namespace jetpwmon

#endif // JETPWMON_PLUS_PLUS_HPP
