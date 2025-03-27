#include <gtest/gtest.h>
#include <jetpwmon/jetpwmon++.hpp> // C++ wrapper header
#include <jetpwmon/jetpwmon.h>     // Include C header for enums like PM_SENSOR_TYPE_* if needed directly
#include <stdexcept>               // For std::runtime_error
#include <thread>                  // For std::this_thread::sleep_for
#include <chrono>                  // For std::chrono::milliseconds
#include <vector>                  // For std::vector
#include <string>                  // For std::string
#include <cstring>                 // For strnlen
#include <cstdio>                  // For potential debug printf

// Test Fixture for C++ API tests
// The fixture itself doesn't need much as RAII handles setup/teardown via the object.
// We might instantiate the object per test, or share one if tests are independent.
// Let's instantiate per test for isolation.
class JetPwMonCPPAPITest : public ::testing::Test
{
protected:
        // Optional: Helper for C string conversion (could be global or in fixture)
        std::string c_char_to_string(const char *c_str, size_t max_len)
        {
                if (!c_str)
                {
                        return "";
                }
                size_t len = strnlen(c_str, max_len);
                return std::string(c_str, len);
        }

        // Helper: Introduce a delay for sampling
        void SleepForSampling(int ms = 200)
        {
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }

        // SetUp() and TearDown() are often not strictly necessary when using RAII
        // unless there's shared state or complex setup beyond object creation.
        void SetUp() override
        {
                // Can add common setup logic here if needed later
        }

        void TearDown() override
        {
                // RAII handles cleanup automatically
        }
};

// Test case: Check successful initialization via constructor (RAII)
TEST_F(JetPwMonCPPAPITest, Initialization)
{
        // Attempt to create the PowerMonitor object.
        // If pm_init fails internally, the constructor should throw.
        ASSERT_NO_THROW({
                jetpwmon::PowerMonitor monitor;
                // Object created successfully, pm_init must have returned PM_SUCCESS.
                // Destructor will automatically call pm_cleanup when 'monitor' goes out of scope.
        }) << "PowerMonitor constructor threw unexpectedly.";
}

// Test case: Setting and getting the sampling frequency using the C++ API
TEST_F(JetPwMonCPPAPITest, SamplingFrequency)
{
        ASSERT_NO_THROW({
                jetpwmon::PowerMonitor monitor;
                int set_freq = 15; // Use a different valid frequency
                int get_freq = 0;

                // Test setting a valid frequency
                ASSERT_NO_THROW(monitor.setSamplingFrequency(set_freq)) << "setSamplingFrequency threw unexpectedly for valid frequency.";

                // Test getting the frequency back
                ASSERT_NO_THROW(get_freq = monitor.getSamplingFrequency()) << "getSamplingFrequency threw unexpectedly.";
                EXPECT_EQ(set_freq, get_freq) << "Got frequency does not match set frequency.";

                // Test setting an invalid frequency (0 Hz), expect std::runtime_error
                EXPECT_THROW(monitor.setSamplingFrequency(0), std::runtime_error) << "Setting frequency to 0 did not throw std::runtime_error.";

                // Test setting another invalid frequency (-5 Hz), expect std::runtime_error
                EXPECT_THROW(monitor.setSamplingFrequency(-5), std::runtime_error) << "Setting frequency to -5 did not throw std::runtime_error.";
        }) << "Test setup failed during PowerMonitor creation";
}

// Test case: Starting, checking status, and stopping sampling using the C++ API
TEST_F(JetPwMonCPPAPITest, SamplingControl)
{
        ASSERT_NO_THROW({
                jetpwmon::PowerMonitor monitor;
                bool is_sampling = false;

                // --- Start Sampling ---
                ASSERT_NO_THROW(monitor.startSampling()) << "startSampling threw unexpectedly.";

                // Check if sampling is active
                ASSERT_NO_THROW(is_sampling = monitor.isSampling()) << "isSampling threw unexpectedly.";
                EXPECT_TRUE(is_sampling) << "isSampling() returned false after starting.";

                // --- Try Starting Again (expect throw) ---
                EXPECT_THROW(monitor.startSampling(), std::runtime_error) << "Starting sampling again did not throw std::runtime_error.";

                // --- Stop Sampling ---
                ASSERT_NO_THROW(monitor.stopSampling()) << "stopSampling threw unexpectedly.";

                // Check if sampling is inactive
                is_sampling = true; // Reset before check
                ASSERT_NO_THROW(is_sampling = monitor.isSampling()) << "isSampling threw unexpectedly after stopping.";
                EXPECT_FALSE(is_sampling) << "isSampling() returned true after stopping.";

                // --- Try Stopping Again (expect throw) ---
                EXPECT_THROW(monitor.stopSampling(), std::runtime_error) << "Stopping sampling again did not throw std::runtime_error.";
        }) << "Test setup failed during PowerMonitor creation";
}

// Test case: Getting the latest instantaneous data using the C++ API
TEST_F(JetPwMonCPPAPITest, DataCollection)
{
        ASSERT_NO_THROW({
                jetpwmon::PowerMonitor monitor;
                jetpwmon::PowerData data = monitor.getLatestData(); // Declare and initialize directly

                // Now perform checks on 'data'
                int count = data.getSensorCount();
                EXPECT_GE(count, 0) << "Sensor count should be non-negative.";

                const pm_sensor_data_t &total_data = data.getTotal();
                EXPECT_GE(total_data.power, 0.0) << "Total power should be non-negative.";
                // Check status string (should be accessible, might be empty)
                EXPECT_NO_THROW({
                        std::string status_str = c_char_to_string(total_data.status, sizeof(total_data.status));
                        // Optionally check content if known, e.g., EXPECT_EQ("OK", status_str);
                });

                // Check sensors array pointer and count consistency via getSensors()
                const pm_sensor_data_t *sensors_ptr = data.getSensors();
                if (count > 0)
                {
                        ASSERT_NE(nullptr, sensors_ptr) << "Sensor count > 0 but sensors pointer is null.";
                        // Check first sensor for basic validity (if sensors exist)
                        const pm_sensor_data_t &first_sensor = sensors_ptr[0];
                        EXPECT_GE(first_sensor.power, 0.0) << "First sensor power should be non-negative.";
                        EXPECT_NO_THROW({
                                std::string name_str = c_char_to_string(first_sensor.name, sizeof(first_sensor.name));
                                EXPECT_FALSE(name_str.empty()) << "First sensor name is empty.";
                        });
                        EXPECT_TRUE(first_sensor.type == PM_SENSOR_TYPE_I2C ||
                                    first_sensor.type == PM_SENSOR_TYPE_SYSTEM ||
                                    first_sensor.type == PM_SENSOR_TYPE_UNKNOWN)
                            << "Invalid sensor type";
                }
                else
                {
                        // If count is 0, pointer could be null or valid (library dependent).
                        // No strict check here unless behavior is guaranteed.
                }
        }) << "Test setup failed during PowerMonitor creation";
}

// Test case: Collecting and checking statistics using the C++ API
TEST_F(JetPwMonCPPAPITest, StatisticsCollection)
{
        ASSERT_NO_THROW({
                jetpwmon::PowerMonitor monitor;

                // 1. Reset statistics
                ASSERT_NO_THROW(monitor.resetStatistics()) << "resetStatistics threw unexpectedly.";

                // 2. Configure and start sampling
                ASSERT_NO_THROW(monitor.setSamplingFrequency(10)) << "setSamplingFrequency threw unexpectedly.";
                ASSERT_NO_THROW(monitor.startSampling()) << "startSampling threw unexpectedly.";

                // 3. Wait for samples
                SleepForSampling(500); // 0.5 seconds

                // 4. Stop sampling
                ASSERT_NO_THROW(monitor.stopSampling()) << "stopSampling threw unexpectedly.";

                // 5. Get statistics
                ASSERT_NO_THROW({
                        jetpwmon::PowerStats stats = monitor.getStatistics();
                }) << "getStatistics threw unexpectedly.";

                // 6. Check statistics structure and validity
                jetpwmon::PowerStats stats = monitor.getStatistics();
                int count = stats.getSensorCount();
                EXPECT_GE(count, 0);

                // Check total stats
                const pm_sensor_stats_t &total_stats = stats.getTotal();
                EXPECT_GT(total_stats.power.count, 0) << "Sample count for total power should be > 0 after sampling.";
                if (total_stats.power.count > 0)
                {
                        EXPECT_LE(total_stats.power.min, total_stats.power.avg);
                        EXPECT_LE(total_stats.power.avg, total_stats.power.max);
                }

                // Check per-sensor stats pointer and data
                const pm_sensor_stats_t *sensors_stats_ptr = stats.getSensors();
                if (count > 0)
                {
                        ASSERT_NE(nullptr, sensors_stats_ptr);
                        // Check first sensor stats
                        const pm_sensor_stats_t &first_sensor_stats = sensors_stats_ptr[0];
                        EXPECT_GT(first_sensor_stats.power.count, 0) << "Sample count for first sensor power should be > 0.";
                        EXPECT_NO_THROW({
                                std::string name_str = c_char_to_string(first_sensor_stats.name, sizeof(first_sensor_stats.name));
                                EXPECT_FALSE(name_str.empty()) << "First sensor name in stats is empty.";
                        });
                        if (first_sensor_stats.power.count > 0)
                        {
                                EXPECT_LE(first_sensor_stats.power.min, first_sensor_stats.power.avg);
                                EXPECT_LE(first_sensor_stats.power.avg, first_sensor_stats.power.max);
                        }
                }
        }) << "Test setup failed during PowerMonitor creation";
}

// Test case: Getting sensor information
TEST_F(JetPwMonCPPAPITest, SensorInfo) {
        ASSERT_NO_THROW({
                jetpwmon::PowerMonitor monitor;
                int count = monitor.getSensorCount();
                EXPECT_GE(count, 0) << "Sensor count should be non-negative.";

                // Test deprecated get_sensor_names function
                #pragma warning(push)
                #pragma warning(disable: 4996)  // Disable deprecation warning for test
                std::vector<std::string> names = monitor.getSensorNames();
                #pragma warning(pop)
                
                EXPECT_EQ(names.size(), static_cast<size_t>(count)) << "Number of names should match sensor count.";
                for (const auto& name : names) {
                        EXPECT_FALSE(name.empty()) << "Sensor name should not be empty.";
                }

                // Test getting sensor names through get_latest_data (recommended way)
                jetpwmon::PowerData data = monitor.getLatestData();
                EXPECT_EQ(data.getSensorCount(), count) << "Sensor count mismatch between direct query and data.";
                
                const pm_sensor_data_t* sensors = data.getSensors();
                if (count > 0) {
                        ASSERT_NE(nullptr, sensors) << "Sensors pointer should not be null when count > 0.";
                        for (int i = 0; i < count; i++) {
                                std::string name = c_char_to_string(sensors[i].name, sizeof(sensors[i].name));
                                EXPECT_FALSE(name.empty()) << "Sensor name from data should not be empty.";
                        }
                }
        }) << "Test setup failed during PowerMonitor creation";
}

// Test case: Check C enum values are accessible (optional, C header needed)
TEST_F(JetPwMonCPPAPITest, SensorTypesEnumCheck)
{
        // These check the underlying C enum values, accessed via the included C header
        EXPECT_EQ(0, PM_SENSOR_TYPE_UNKNOWN);
        EXPECT_EQ(1, PM_SENSOR_TYPE_I2C);
        EXPECT_EQ(2, PM_SENSOR_TYPE_SYSTEM);
}

// Main function to run all Google Tests
// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
// Note: If using GTest::gtest_main, this main function is not needed.