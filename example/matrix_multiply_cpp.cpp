#include <jetpwmon/jetpwmon++.hpp> // C++ wrapper header for jetpwmon
#include <iostream>                // For console output (cout, cerr)
#include <vector>                  // Although not strictly needed, common include
#include <string>                  // For std::string
#include <thread>                  // For std::this_thread::sleep_for
#include <chrono>                  // For timing (high_resolution_clock, duration)
#include <stdexcept>               // For std::runtime_error exception handling
#include <Eigen/Dense>             // For Eigen matrix library
#include <cstring>                 // For strnlen (safe C string length calculation)

// --- Configuration Constants ---

/**
 * @brief Matrix dimension for the Eigen task. Adjust based on system performance and memory.
 */
constexpr int MATRIX_SIZE = 1000; // A moderate size for demonstration

/**
 * @brief Number of iterations in the matrix multiplication loop to increase workload.
 */
constexpr int NUM_ITERATIONS = 20;

// --- Helper Function ---

/**
 * @brief Safely converts a C-style char array (potentially not null-terminated within max_len)
 * to a std::string.
 * @param c_str Pointer to the C char array.
 * @param max_len The maximum number of characters to check (typically sizeof the source array).
 * @return std::string containing the characters up to the first null terminator or max_len.
 * Returns an empty string if c_str is null.
 */
std::string c_char_to_string(const char *c_str, size_t max_len)
{
        if (!c_str)
        {
                return ""; // Handle null pointer case
        }
        // Find the actual length, constrained by max_len
        size_t len = strnlen(c_str, max_len);
        return std::string(c_str, len);
}

// --- CPU-Intensive Task ---

/**
 * @brief Performs a CPU-intensive task using Eigen matrix multiplication.
 * Eigen may utilize multiple cores automatically depending on its configuration and build settings.
 */
void eigen_cpu_task()
{
        std::cout << "Starting Eigen CPU-intensive task (Matrix Size: "
                  << MATRIX_SIZE << "x" << MATRIX_SIZE << ", Iterations: " << NUM_ITERATIONS << ")..." << std::endl;

        // Initialize two square matrices with random values.
        // Eigen::MatrixXd is a matrix of doubles with dynamic size.
        Eigen::MatrixXd a = Eigen::MatrixXd::Random(MATRIX_SIZE, MATRIX_SIZE);
        Eigen::MatrixXd b = Eigen::MatrixXd::Random(MATRIX_SIZE, MATRIX_SIZE);

        // Perform repeated matrix multiplications to generate load.
        for (int i = 0; i < NUM_ITERATIONS; ++i)
        {
                // Eigen efficiently handles the multiplication.
                // Reassigning 'a' is a simple way to continue the workload.
                a = a * b;
        }

        std::cout << "Eigen CPU-intensive task finished." << std::endl;
}

// --- Main Program ---

int main()
{
        try
        {
                // === 1. Initialize Power Monitor (RAII) ===
                // Create the PowerMonitor object. Its constructor calls pm_init().
                // If pm_init fails, it throws std::runtime_error.
                // Cleanup (pm_cleanup) is guaranteed via the object's destructor.
                jetpwmon::PowerMonitor monitor;
                std::cout << "Power monitor initialized successfully." << std::endl;

                // === 2. Configure Sampling (Optional but recommended) ===
                int frequency = 1000;                    // Target sampling rate in Hz
                monitor.setSamplingFrequency(frequency); // Throws on error
                std::cout << "Set sampling frequency to " << frequency << " Hz." << std::endl;

                // === 3. Reset Statistics ===
                // Clear any previous statistics before starting the measurement period.
                monitor.resetStatistics(); // Throws on error
                std::cout << "Statistics reset." << std::endl;

                // === 4. Start Background Sampling ===
                // This starts the C library's background thread for data collection.
                monitor.startSampling(); // Throws on error
                std::cout << "Started power sampling..." << std::endl;

                // === 5. Execute the CPU-Intensive Task ===
                // Record time just before and after the task.
                auto task_start_time = std::chrono::high_resolution_clock::now();

                eigen_cpu_task(); // Run the task on the main thread

                auto task_end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> task_duration = task_end_time - task_start_time;
                std::cout << "\nTask execution finished in: " << task_duration.count() << " seconds" << std::endl;

                // === 6. Short Delay (Optional) ===
                // Allow a brief moment for the sampling thread to capture the state immediately after the task finishes.
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100 ms

                // === 7. Stop Background Sampling ===
                // Signals the C library's background thread to stop collecting data.
                monitor.stopSampling(); // Throws on error
                std::cout << "Stopped power sampling." << std::endl;

                // === 8. Get Collected Statistics ===
                // Retrieves the statistics accumulated during the sampling period.
                // Returns a jetpwmon::PowerStats object.
                jetpwmon::PowerStats stats = monitor.getStatistics(); // Throws on error

                // === 9. Process and Print Statistics ===
                std::cout << "\n--- Power Consumption Statistics ---" << std::endl;

                // Access the aggregated total statistics.
                const pm_sensor_stats_t &total_stats = stats.getTotal();
                double task_duration_sec = task_duration.count(); // Duration in seconds

                // Calculate energy based on the measured average power and task duration.
                // Note: The 'total_stats.power.total' field contains energy calculated by the library,
                // potentially based on sample count and frequency, which might be slightly different.
                double calculated_total_energy = total_stats.power.avg * task_duration_sec;

                std::cout << "Total Power Consumption:" << std::endl;
                std::cout << "  Min Power   : " << total_stats.power.min << " W" << std::endl;
                std::cout << "  Max Power   : " << total_stats.power.max << " W" << std::endl;
                std::cout << "  Avg Power   : " << total_stats.power.avg << " W" << std::endl;
                std::cout << "  Total Energy: " << calculated_total_energy << " J (Avg * Measured Duration)" << std::endl;
                std::cout << "  (Lib Energy): " << total_stats.power.total << " J" << std::endl; // Display library's calculation too
                std::cout << "  Sample Count: " << total_stats.power.count << std::endl;

                // Access per-sensor statistics using the pointer and count.
                std::cout << "\nPer-Sensor Power Consumption:" << std::endl;
                const pm_sensor_stats_t *sensors_stats_ptr = stats.getSensors(); // Raw C pointer
                int sensor_count = stats.getSensorCount();

                // SAFETY NOTE: The pointer from getSensors() is managed by the C library and
                // is likely valid only until the next monitor call or destruction. Access promptly.
                if (sensors_stats_ptr != nullptr && sensor_count > 0)
                {
                        for (int i = 0; i < sensor_count; ++i)
                        {
                                const pm_sensor_stats_t &sensor_stat = sensors_stats_ptr[i]; // Access element
                                double calculated_sensor_energy = sensor_stat.power.avg * task_duration_sec;

                                // Safely convert the C char array name to std::string
                                std::cout << "\n  Sensor: " << c_char_to_string(sensor_stat.name, sizeof(sensor_stat.name)) << std::endl;
                                std::cout << "    Min Power   : " << sensor_stat.power.min << " W" << std::endl;
                                std::cout << "    Max Power   : " << sensor_stat.power.max << " W" << std::endl;
                                std::cout << "    Avg Power   : " << sensor_stat.power.avg << " W" << std::endl;
                                std::cout << "    Total Energy: " << calculated_sensor_energy << " J (Avg * Measured Duration)" << std::endl;
                                std::cout << "    (Lib Energy): " << sensor_stat.power.total << " J" << std::endl;
                                std::cout << "    Sample Count: " << sensor_stat.power.count << std::endl;
                        }
                }
                else
                {
                        std::cout << "  No per-sensor statistics available." << std::endl;
                }

                // === 10. Cleanup (Automatic) ===
                std::cout << "\nProgram finished. PowerMonitor resources automatically released by RAII." << std::endl;
                // The 'monitor' object goes out of scope here, triggering its destructor, which calls pm_cleanup().
        }
        catch (const std::runtime_error &e)
        {
                // Catch exceptions thrown by the PowerMonitor C++ wrapper on C API errors.
                std::cerr << "ERROR: " << e.what() << std::endl;
                return 1; // Indicate failure
        }
        catch (const std::exception &e)
        {
                // Catch other potential standard library exceptions.
                std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
                return 1;
        }

        return 0; // Indicate success
}