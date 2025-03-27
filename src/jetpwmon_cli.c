/**
 * @file jetpwmon_cli.c
 * @brief Dynamic command line interface for the power monitor library using ncurses.
 */

#define _POSIX_C_SOURCE 200809L // Needed for clock_gettime and nanosleep
// #define _XOPEN_SOURCE 500 // No longer strictly needed if only using nanosleep

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // For getopt
#include <signal.h>
#include <time.h>       // For clock_gettime, nanosleep, struct timespec
#include <ncurses.h>    // For terminal UI
#include <locale.h>     // For setlocale (UTF-8 support)
#include <math.h>       // For round() if needed, or just integer division
#include "jetpwmon/jetpwmon.h"

// --- Constants ---
#define MAX_REFRESH_HZ 30 // Maximum screen refresh rate in Hz
const int MIN_INTERVAL_MS = (1000 / MAX_REFRESH_HZ); // Minimum interval in ms (~33ms)

// --- Global Variables ---
static pm_handle_t g_handle = NULL;
volatile sig_atomic_t g_terminate_flag = 0;
static int update_count = 0; // Simple counter for visual feedback

// --- Signal Handler ---
static void signal_handler(int signum) {
    (void)signum;
    g_terminate_flag = 1;
}

// --- ncurses UI Function ---
// (No changes needed in draw_ui from the previous version, assuming it worked)
static void draw_ui(pm_handle_t handle, int freq, double elapsed_sec)
{
    pm_error_t err;
    pm_power_data_t data;
    int row = 0;
    int col = 0;
    char buffer[256];

    // 1. Get Latest Data (Make sure C API memory is fixed!)
    err = pm_get_latest_data(handle, &data);
    if (err != PM_SUCCESS)
    {
        // Be careful with ncurses functions if called rapidly after endwin potentially
        // For simplicity, just note the error might overwrite previous screen partially
        mvprintw(row++, col, "Error getting data: %s", pm_error_string(err));
        refresh();
        // Use nanosleep instead of usleep
        struct timespec error_sleep = {0, 500 * 1000 * 1000}; // 0.5 seconds
        nanosleep(&error_sleep, NULL);
        return;
    }

    // 2. Clear Screen and add counter
    clear();
    mvprintw(0, COLS - 15, "Update: %d", update_count++);

    // 3. Print Header Info
    attron(A_BOLD);
    snprintf(buffer, sizeof(buffer), "Jetson Power Monitor (Sampling: %d Hz, Elapsed: %.1f s) - Press 'q' to quit",
             freq, elapsed_sec);
    mvprintw(row++, col, "%s", buffer);
    attroff(A_BOLD);
    row++;

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
            const pm_sensor_data_t *sensor = &data.sensors[i];
            mvprintw(row++, col, "%-18s %10.2f %10.2f %10.2f %10s %-10s",
                     sensor->name, sensor->power, sensor->voltage, sensor->current,
                     sensor->online ? "Yes" : "No", sensor->status);
        }
    }
    else
    {
        mvprintw(row++, col, "No individual sensor data available.");
    }

    // 7. Refresh Screen (check for errors)
    if (refresh() == ERR) {
         mvprintw(LINES - 1, 0, "Error: ncurses refresh() failed!");
    }
}


// --- Usage Function ---
static void print_usage(const char *prog_name) {
    printf("Usage: %s [-f frequency_hz] [-d duration_seconds] [-i interval_ms]\n", prog_name);
    printf("  -f frequency_hz     Sampling frequency for the library (Hz, default: 1)\n");
    printf("  -d duration_seconds Monitoring duration (seconds, 0 for indefinite, default: 0)\n");
    printf("  -i interval_ms      Screen refresh interval (ms, default: 1000, min: ~%dms for %dHz)\n", MIN_INTERVAL_MS, MAX_REFRESH_HZ);
    printf("  -h                  Show this help message\n");
}

// --- Main Function ---
int main(int argc, char* argv[]) {
    pm_error_t error;
    int sampling_frequency = 50; // Library sampling frequency
    int duration = 0;
    int update_interval_ms = 1000; // Screen refresh interval
    int opt;

    // --- Parse Arguments ---
    while ((opt = getopt(argc, argv, "f:d:i:h")) != -1) {
        switch (opt) {
            case 'f': sampling_frequency = atoi(optarg); break;
            case 'd': duration = atoi(optarg); break;
            case 'i': update_interval_ms = atoi(optarg); break;
            case 'h': print_usage(argv[0]); return 0;
            default: print_usage(argv[0]); return 1;
        }
    }
    // Validate inputs
    if (sampling_frequency <= 0) { fprintf(stderr, "Error: Sampling frequency must be positive.\n"); return 1; }
    if (duration < 0) { fprintf(stderr, "Error: Duration cannot be negative.\n"); return 1; }
    if (update_interval_ms <= 0) { fprintf(stderr, "Error: Update interval must be positive.\n"); return 1; }

    // --- Apply Refresh Rate Cap ---
    if (update_interval_ms < MIN_INTERVAL_MS) {
        printf("Note: Requested update interval %dms is faster than %dHz cap. Using ~%dms.\n",
               update_interval_ms, MAX_REFRESH_HZ, MIN_INTERVAL_MS);
        update_interval_ms = MIN_INTERVAL_MS;
    }

    // --- Signal Handling ---
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // --- Initialize Library ---
    printf("Initializing power monitor...\n");
    error = pm_init(&g_handle);
    if (error != PM_SUCCESS) { /* Error handling */ fprintf(stderr, "Init Error: %s\n", pm_error_string(error)); return 1; }
    printf("Initialization successful.\n");

    // --- Set Sampling Frequency ---
    printf("Setting sampling frequency to %d Hz...\n", sampling_frequency);
    error = pm_set_sampling_frequency(g_handle, sampling_frequency);
    if (error != PM_SUCCESS) { /* Error handling */ fprintf(stderr, "Freq Error: %s\n", pm_error_string(error)); pm_cleanup(g_handle); return 1; }

    // --- Initialize ncurses ---
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(update_interval_ms); // Set blocking timeout for getch() in ms

    // --- Start Sampling Thread ---
    error = pm_start_sampling(g_handle);
    if (error != PM_SUCCESS) { /* Error handling */ endwin(); fprintf(stderr, "Start Error: %s\n", pm_error_string(error)); pm_cleanup(g_handle); return 1; }

    // --- Main Monitoring Loop ---
    struct timespec start_ts, current_ts;
    clock_gettime(CLOCK_MONOTONIC, &start_ts);
    double elapsed_seconds = 0;

    while (!g_terminate_flag) {
        // Calculate elapsed time
        clock_gettime(CLOCK_MONOTONIC, &current_ts);
        elapsed_seconds = (current_ts.tv_sec - start_ts.tv_sec) +
                          (current_ts.tv_nsec - start_ts.tv_nsec) / 1e9;

        // Check duration limit
        if (duration > 0 && elapsed_seconds >= duration) { break; }

        // Draw UI (fetches data inside)
        draw_ui(g_handle, sampling_frequency, elapsed_seconds);

        // Wait for input or timeout
        int ch = getch();
        if (ch == 'q' || ch == 'Q') { break; }
        // If ch == ERR, timeout occurred, loop continues normally.
    }

    // --- Stop Sampling ---
    error = pm_stop_sampling(g_handle);
    // Error check for stop_sampling (optional, depends on desired strictness)

    // --- Cleanup ncurses ---
    endwin(); // Restore terminal

    // --- Final Cleanup (C Library) ---
    printf("Cleaning up resources...\n");
    error = pm_cleanup(g_handle); // Error check is good
    if (error != PM_SUCCESS) { fprintf(stderr, "Cleanup Error: %s\n", pm_error_string(error)); return 1; }

    // --- Final Messages ---
    printf("Power monitoring stopped.\n");
    // ... (Print final duration/interruption message as before) ...

    return 0;
}