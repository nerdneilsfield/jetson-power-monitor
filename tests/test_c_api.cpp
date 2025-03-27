#include <gtest/gtest.h>
#include <jetpwmon/jetpwmon.h> // C API header
#include <thread>              // For std::this_thread::sleep_for
#include <chrono>              // For std::chrono::milliseconds
#include <vector>              // Can be useful, though not strictly required here
#include <string>              // For checking error strings
#include <cstdio>              // For potential debug printf

// Test Fixture for managing pm_handle_t lifecycle
class JetPwMonCAPITest : public ::testing::Test {
protected:
    pm_handle_t handle_ = nullptr; // Handle for the library instance

    // Per-test-suite set-up: Might be needed for global setup once
    // static void SetUpTestSuite() {}

    // Per-test set-up: Initialize library before each test
    void SetUp() override {
        pm_error_t err = pm_init(&handle_);
        // If initialization fails, skip subsequent tests as they depend on the handle.
        // Use ASSERT_EQ for critical setup.
        ASSERT_EQ(PM_SUCCESS, err) << "Failed to initialize jetpwmon: " << pm_error_string(err);
        ASSERT_NE(nullptr, handle_) << "Initialization succeeded but handle is null.";
    }

    // Per-test tear-down: Cleanup library after each test
    void TearDown() override {
        if (handle_ != nullptr) {
            pm_error_t err = pm_cleanup(handle_);
            // Use EXPECT_EQ here so other potential teardown issues can be seen if cleanup fails.
            EXPECT_EQ(PM_SUCCESS, err) << "Failed to cleanup jetpwmon: " << pm_error_string(err);
            handle_ = nullptr; // Ensure handle is null after cleanup attempt
        }
    }

    // Helper: Introduce a delay for sampling
    void SleepForSampling(int ms = 200) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
};

// Test case: Check basic initialization (covered by SetUp) and cleanup (covered by TearDown)
// This test primarily ensures the fixture itself works.
TEST_F(JetPwMonCAPITest, InitializationAndCleanup) {
    // SetUp already called pm_init and asserted success.
    // TearDown will call pm_cleanup and check its success.
    SUCCEED(); // Indicate the fixture setup/teardown represents the test
}

// Test case: Setting and getting the sampling frequency
TEST_F(JetPwMonCAPITest, SamplingFrequency) {
    int set_freq = 10; // Hz - Use a low, likely valid frequency
    int get_freq = 0;
    pm_error_t err;

    // Test setting a valid frequency
    err = pm_set_sampling_frequency(handle_, set_freq);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to set valid sampling frequency: " << pm_error_string(err);

    // Test getting the frequency back
    err = pm_get_sampling_frequency(handle_, &get_freq);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to get sampling frequency: " << pm_error_string(err);
    EXPECT_EQ(set_freq, get_freq) << "Got frequency does not match set frequency.";

    // Test setting an invalid frequency (0 Hz)
    err = pm_set_sampling_frequency(handle_, 0);
    EXPECT_EQ(PM_ERROR_INVALID_FREQUENCY, err) << "Setting frequency to 0 did not return expected error.";

    // Test setting another invalid frequency (-1 Hz)
    err = pm_set_sampling_frequency(handle_, -1);
    EXPECT_EQ(PM_ERROR_INVALID_FREQUENCY, err) << "Setting frequency to -1 did not return expected error.";
}

// Test case: Starting, checking status, and stopping sampling
TEST_F(JetPwMonCAPITest, SamplingControl) {
    pm_error_t err;
    bool is_sampling = false;

    // --- Start Sampling ---
    err = pm_start_sampling(handle_);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to start sampling: " << pm_error_string(err);

    // Check if sampling is active
    err = pm_is_sampling(handle_, &is_sampling);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to check sampling status: " << pm_error_string(err);
    EXPECT_TRUE(is_sampling) << "is_sampling returned false after starting.";

    // --- Try Starting Again (expect error) ---
    err = pm_start_sampling(handle_);
    EXPECT_EQ(PM_ERROR_ALREADY_RUNNING, err) << "Starting sampling again did not return expected error.";

    // --- Stop Sampling ---
    err = pm_stop_sampling(handle_);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to stop sampling: " << pm_error_string(err);

    // Check if sampling is inactive
    is_sampling = true; // Reset variable before checking
    err = pm_is_sampling(handle_, &is_sampling);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to check sampling status after stopping: " << pm_error_string(err);
    EXPECT_FALSE(is_sampling) << "is_sampling returned true after stopping.";

    // --- Try Stopping Again (expect error) ---
    err = pm_stop_sampling(handle_);
    EXPECT_EQ(PM_ERROR_NOT_RUNNING, err) << "Stopping sampling again did not return expected error.";
}

// Test case: Getting the latest instantaneous data
TEST_F(JetPwMonCAPITest, DataCollection) {
    pm_error_t err;
    pm_power_data_t data;

    // Optional: Start and stop sampling briefly just to ensure sensors are read at least once.
    // Alternatively, pm_get_latest_data might work even without sampling running.
    // Let's assume it works without active sampling for this test. If not, add start/stop.
    // err = pm_start_sampling(handle_);
    // ASSERT_EQ(PM_SUCCESS, err);
    // SleepForSampling(100); // Short wait
    // err = pm_stop_sampling(handle_);
    // ASSERT_EQ(PM_SUCCESS, err);

    err = pm_get_latest_data(handle_, &data);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to get latest data: " << pm_error_string(err);

    // Basic checks on the returned structure
    EXPECT_GE(data.sensor_count, 0) << "Sensor count should be non-negative.";

    // Check total data (values depend on hardware state, just check bounds/validity)
    EXPECT_GE(data.total.power, 0.0) << "Total power should be non-negative.";
    // Voltage/Current can be tricky, depends on sensor setup. Maybe just check presence.

    // Check sensors array pointer and count consistency
    if (data.sensor_count > 0) {
        ASSERT_NE(nullptr, data.sensors) << "Sensor count > 0 but sensors pointer is null.";
        // Check first sensor for basic validity (if sensors exist)
        EXPECT_GE(data.sensors[0].power, 0.0) << "First sensor power should be non-negative.";
        // Check if name is reasonably populated (at least first char not null if expected)
        if (strlen(data.sensors[0].name) == 0 && data.sensor_count > 0) {
             // This might be okay if names aren't guaranteed, or might be a failure sign
             // GTEST_LOG_(INFO) << "Warning: First sensor name is empty.";
        } else if (data.sensor_count > 0) {
             EXPECT_NE('\0', data.sensors[0].name[0]) << "First sensor name starts with null terminator.";
        }
    } else {
        // If count is 0, pointer might be null or not, depending on C library implementation.
        // EXPECT_EQ(nullptr, data.sensors) << "Sensor count is 0 but pointer is not null."; // This might be too strict
    }
}

// Test case: Collecting and checking statistics
TEST_F(JetPwMonCAPITest, StatisticsCollection) {
    pm_error_t err;
    pm_power_stats_t stats;

    // 1. Reset statistics
    err = pm_reset_statistics(handle_);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to reset statistics: " << pm_error_string(err);

    // 2. Configure and start sampling
    err = pm_set_sampling_frequency(handle_, 10); // Low freq is fine for testing stats mechanism
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to set sampling freq for stats: " << pm_error_string(err);
    err = pm_start_sampling(handle_);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to start sampling for stats: " << pm_error_string(err);

    // 3. Wait for samples to be collected
    SleepForSampling(500); // Wait 0.5 seconds

    // 4. Stop sampling
    err = pm_stop_sampling(handle_);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to stop sampling for stats: " << pm_error_string(err);

    // 5. Get statistics
    err = pm_get_statistics(handle_, &stats);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to get statistics: " << pm_error_string(err);

    // 6. Check statistics structure and basic validity
    EXPECT_GE(stats.sensor_count, 0);

    // Check total stats
    EXPECT_GT(stats.total.power.count, 0) << "Sample count for total power should be > 0 after sampling.";
    if (stats.total.power.count > 0) {
        EXPECT_LE(stats.total.power.min, stats.total.power.avg) << "Min <= Avg failed for total power.";
        EXPECT_LE(stats.total.power.avg, stats.total.power.max) << "Avg <= Max failed for total power.";
        // Check other stats if needed (voltage, current)
    }

    // Check per-sensor stats pointer and count
    if (stats.sensor_count > 0) {
        ASSERT_NE(nullptr, stats.sensors);
        // Check first sensor stats
        EXPECT_GT(stats.sensors[0].power.count, 0) << "Sample count for first sensor power should be > 0.";
         if (strlen(stats.sensors[0].name) == 0) {
             // GTEST_LOG_(INFO) << "Warning: First sensor name in stats is empty.";
         } else {
             EXPECT_NE('\0', stats.sensors[0].name[0]);
         }
        if (stats.sensors[0].power.count > 0) {
            EXPECT_LE(stats.sensors[0].power.min, stats.sensors[0].power.avg);
            EXPECT_LE(stats.sensors[0].power.avg, stats.sensors[0].power.max);
            // Check other stats if needed
        }
    }
}

// Test case: Sensor information retrieval (count)
TEST_F(JetPwMonCAPITest, SensorInfo) {
    pm_error_t err;
    int count = -1; // Initialize to invalid value

    err = pm_get_sensor_count(handle_, &count);
    ASSERT_EQ(PM_SUCCESS, err) << "Failed to get sensor count: " << pm_error_string(err);
    EXPECT_GE(count, 0) << "Sensor count should be non-negative.";

    // Optional: Compare with counts from data/stats if available
    // pm_power_data_t data;
    // err = pm_get_latest_data(handle_, &data);
    // if (err == PM_SUCCESS) {
    //     EXPECT_EQ(count, data.sensor_count) << "Sensor count mismatch between get_sensor_count and get_latest_data.";
    // }
}

// Test case: Error handling functions
TEST_F(JetPwMonCAPITest, ErrorHandling) {
    // Test getting string for success code
    const char* success_msg = pm_error_string(PM_SUCCESS);
    ASSERT_NE(nullptr, success_msg) << "pm_error_string(PM_SUCCESS) returned null.";
    EXPECT_GT(strlen(success_msg), 0) << "pm_error_string(PM_SUCCESS) returned empty string.";
    // Optional: Check specific content
    // EXPECT_STREQ("Success", success_msg); // Depends on exact string definition

    // Test getting string for a known error code
    const char* init_fail_msg = pm_error_string(PM_ERROR_INIT_FAILED);
    ASSERT_NE(nullptr, init_fail_msg) << "pm_error_string(PM_ERROR_INIT_FAILED) returned null.";
    EXPECT_GT(strlen(init_fail_msg), 0) << "pm_error_string(PM_ERROR_INIT_FAILED) returned empty string.";
    EXPECT_STRNE(success_msg, init_fail_msg) << "Error string for SUCCESS and INIT_FAILED are the same.";

    // Test getting string for an potentially unknown code (use a large negative number)
    const char* unknown_msg = pm_error_string((pm_error_t)-999);
     ASSERT_NE(nullptr, unknown_msg) << "pm_error_string for unknown code returned null.";
     EXPECT_GT(strlen(unknown_msg), 0) << "pm_error_string for unknown code returned empty string.";
}

// Test case: Check enum values for sensor types
TEST_F(JetPwMonCAPITest, SensorTypesEnum) {
    EXPECT_EQ(0, PM_SENSOR_TYPE_UNKNOWN);
    EXPECT_EQ(1, PM_SENSOR_TYPE_I2C);
    EXPECT_EQ(2, PM_SENSOR_TYPE_SYSTEM);
    // Add checks if more types are defined
}


// Main function to run all tests
// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }