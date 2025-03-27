/**
 * @file jetpwmon_cli.c
 * @brief Dynamic command line interface for the power monitor library using ncurses.
 */

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For usleep, getopt
#include <signal.h>
#include <time.h>    // For clock_gettime
#include <ncurses.h> // For terminal UI
#include <locale.h>  // For setlocale (UTF-8 support)
#include "jetpwmon/jetpwmon.h"

// --- Global Variables ---
static pm_handle_t g_handle = NULL;         // Global handle for signal handler cleanup
volatile sig_atomic_t g_terminate_flag = 0; // Signal flag for graceful shutdown
static int update_count = 0;
// --- Signal Handler ---
static void signal_handler(int signum)
{
        (void)signum;         // Unused parameter
        g_terminate_flag = 1; // Set flag to request termination
}

// --- ncurses UI Function ---
static void draw_ui(pm_handle_t handle, int freq, double elapsed_sec)
{
        pm_error_t err;
        pm_power_data_t data;
        int row = 0;
        int col = 0;
        char buffer[256]; // Buffer for formatted strings

        // 1. Get Latest Data (MUST use the fixed C API version)
        // Assuming API Option A: data.sensors points to internal buffer
        err = pm_get_latest_data(handle, &data);
        if (err != PM_SUCCESS)
        {
                mvprintw(row++, col, "Error getting data: %s", pm_error_string(err));
                refresh();
                usleep(500000); // Show error briefly
                return;
        }

        // 2. Clear Screen
        clear();
        mvprintw(0, COLS - 15, "Update: %d", update_count++); // 在右上角打印计数器

        // 3. Print Header Info
        attron(A_BOLD); // Turn on bold attribute
        snprintf(buffer, sizeof(buffer), "Jetson Power Monitor (Freq: %d Hz, Elapsed: %.1f s) - Press 'q' to quit",
                 freq, elapsed_sec);
        mvprintw(row++, col, "%s", buffer);
        attroff(A_BOLD); // Turn off bold attribute
        row++;           // Add a blank line

        // 4. Print Table Header
        attron(A_UNDERLINE);
        mvprintw(row++, col, "%-18s %10s %10s %10s %10s %-10s",
                 "Sensor Name", "Power (W)", "Voltage(V)", "Current(A)", "Online", "Status");
        attroff(A_UNDERLINE);

        // 5. Print Total Data Row
        mvprintw(row++, col, "%-18s %10.2f %10.2f %10.2f %10s %-10s",
                 data.total.name,
                 data.total.power,
                 data.total.voltage,
                 data.total.current,
                 data.total.online ? "Yes" : "No",
                 data.total.status);

        // 6. Print Individual Sensor Rows
        if (data.sensors != NULL && data.sensor_count > 0)
        {
                for (int i = 0; i < data.sensor_count; i++)
                {
                        const pm_sensor_data_t *sensor = &data.sensors[i]; // Access via pointer
                        mvprintw(row++, col, "%-18s %10.2f %10.2f %10.2f %10s %-10s",
                                 sensor->name, // Assumes null-terminated
                                 sensor->power,
                                 sensor->voltage,
                                 sensor->current,
                                 sensor->online ? "Yes" : "No",
                                 sensor->status // Assumes null-terminated
                        );
                }
        }
        else
        {
                mvprintw(row++, col, "No individual sensor data available.");
        }

        // 7. Refresh Screen
        if (refresh() == ERR)
        {
                // If refresh fails, subsequent calls might not work either.
                // Maybe try to log this error somewhere persistently?
                // For now, just be aware. You could print to a fixed bottom line.
                mvprintw(LINES - 1, 0, "Error: ncurses refresh() failed!");
                // Note: This message itself might not appear if refresh is fundamentally broken.
        }
}

// --- Usage Function ---
static void print_usage(const char *prog_name)
{
        printf("Usage: %s [-f frequency_hz] [-d duration_seconds] [-i interval_ms]\n", prog_name);
        printf("  -f frequency_hz     Sampling frequency in Hz (default: 1)\n");
        printf("  -d duration_seconds Monitoring duration in seconds (default: indefinite)\n");
        printf("  -i interval_ms      Screen update interval in milliseconds (default: 1000)\n");
        printf("  -h                  Show this help message\n");
}

// --- Main Function ---
int main(int argc, char *argv[])
{
        pm_error_t error;
        int sampling_frequency = 1;
        int duration = 0;              // 0 means run indefinitely
        int update_interval_ms = 1000; // Default screen update interval
        int opt;

        // --- Parse Command Line Arguments using getopt ---
        while ((opt = getopt(argc, argv, "f:d:i:h")) != -1)
        {
                switch (opt)
                {
                case 'f':
                        sampling_frequency = atoi(optarg);
                        if (sampling_frequency <= 0)
                        {
                                fprintf(stderr, "Error: Sampling frequency must be positive.\n");
                                return 1;
                        }
                        break;
                case 'd':
                        duration = atoi(optarg);
                        if (duration < 0)
                        {
                                fprintf(stderr, "Error: Duration cannot be negative.\n");
                                return 1;
                        }
                        break;
                case 'i':
                        update_interval_ms = atoi(optarg);
                        if (update_interval_ms <= 0)
                        {
                                fprintf(stderr, "Error: Update interval must be positive.\n");
                                return 1;
                        }
                        break;
                case 'h':
                        print_usage(argv[0]);
                        return 0;
                default: /* '?' */
                        print_usage(argv[0]);
                        return 1;
                }
        }

        // --- Set up Signal Handling ---
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = signal_handler;
        sigaction(SIGINT, &sa, NULL);  // Handle Ctrl+C
        sigaction(SIGTERM, &sa, NULL); // Handle termination signal

        // --- Initialize Library ---
        printf("Initializing power monitor...\n");
        error = pm_init(&g_handle); // Use global handle
        if (error != PM_SUCCESS)
        {
                fprintf(stderr, "Error initializing power monitor: %s\n", pm_error_string(error));
                return 1;
        }
        printf("Initialization successful.\n");

        // --- Set Frequency ---
        printf("Setting sampling frequency to %d Hz...\n", sampling_frequency);
        error = pm_set_sampling_frequency(g_handle, sampling_frequency);
        if (error != PM_SUCCESS)
        {
                fprintf(stderr, "Error setting sampling frequency: %s\n", pm_error_string(error));
                pm_cleanup(g_handle); // Cleanup before exiting
                return 1;
        }

        // --- Initialize ncurses ---
        setlocale(LC_ALL, ""); // Support UTF-8 etc.
        initscr();             // Start ncurses mode
        cbreak();              // Disable line buffering
        noecho();              // Don't echo input chars
        keypad(stdscr, TRUE);  // Enable F-keys, arrows etc.
        curs_set(0);           // Hide cursor
        timeout(0);            // Make getch() non-blocking

        // --- Start Sampling Thread ---
        printf("Starting power monitoring...\n"); // This message might be overwritten by ncurses
        error = pm_start_sampling(g_handle);
        if (error != PM_SUCCESS)
        {
                endwin(); // Restore terminal before printing error
                fprintf(stderr, "Error starting power monitoring: %s\n", pm_error_string(error));
                pm_cleanup(g_handle);
                return 1;
        }

        // --- Main Monitoring Loop ---
        struct timespec start_ts, current_ts;
        clock_gettime(CLOCK_MONOTONIC, &start_ts);
        double elapsed_seconds = 0;
        useconds_t update_interval_us = update_interval_ms * 1000;

        while (!g_terminate_flag)
        {
                // Calculate elapsed time
                clock_gettime(CLOCK_MONOTONIC, &current_ts);
                elapsed_seconds = (current_ts.tv_sec - start_ts.tv_sec) +
                                  (current_ts.tv_nsec - start_ts.tv_nsec) / 1e9;

                // Check duration limit
                if (duration > 0 && elapsed_seconds >= duration)
                {
                        break; // Exit loop if duration reached
                }

                // Draw the UI
                draw_ui(g_handle, sampling_frequency, elapsed_seconds); // Call drawing function

                int ch = getch(); // Wait up to 'update_interval_ms' for input
                if (ch == 'q' || ch == 'Q')
                {
                        break; // Exit loop on 'q'
                }
                else if (ch == ERR)
                {
                        // Timeout occurred, no input, continue loop normally
                }
                else
                {
                        // Handle other keys if needed, or ignore
                }
        }

        // --- Stop Sampling ---
        // Print message *before* endwin if possible, though it might get cleared.
        // move(LINES - 1, 0); // Move cursor to last line
        // printw("Stopping sampling...");
        // refresh(); // Show stopping message briefly
        error = pm_stop_sampling(g_handle);
        if (error != PM_SUCCESS && error != PM_ERROR_NOT_RUNNING)
        { // Ignore error if already stopped
                // Need to end ncurses before printing error to stderr
                endwin();
                fprintf(stderr, "Warning: Error stopping power monitoring: %s\n", pm_error_string(error));
                // Continue to cleanup
        }

        // --- Cleanup ncurses ---
        endwin(); // Restore terminal settings *before* final prints/exit

        // --- Final Cleanup (C Library) ---
        printf("Cleaning up resources...\n");
        error = pm_cleanup(g_handle);
        if (error != PM_SUCCESS)
        {
                fprintf(stderr, "Error cleaning up: %s\n", pm_error_string(error));
                return 1; // Return error code if cleanup fails
        }

        printf("Power monitoring stopped.\n");
        if (duration > 0 && elapsed_seconds < duration && g_terminate_flag)
        {
                printf("Monitoring interrupted by signal after %.1f seconds.\n", elapsed_seconds);
        }
        else if (duration > 0)
        {
                printf("Monitoring completed after %d seconds.\n", duration);
        }
        else
        {
                printf("Monitoring stopped after %.1f seconds.\n", elapsed_seconds);
        }

        return 0; // Success
}