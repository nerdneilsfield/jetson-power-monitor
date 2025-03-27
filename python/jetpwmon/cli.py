import argparse
import signal
import sys
import time
import math
from typing import Optional

# Try importing rich, provide helpful error if missing
try:
    from rich.live import Live
    from rich.table import Table
    from rich.console import Console
except ImportError:
    print("Error: 'rich' library not found.", file=sys.stderr)
    print("Please install it to run the CLI: pip install rich", file=sys.stderr)
    # Or suggest: pip install jetpwmon[cli] if using optional dependencies
    sys.exit(1)

# Import the core pybind11 module
# Assumes __init__.py makes PowerMonitor available
try:
    # Adjust import based on your package structure if needed
    from . import PowerMonitor  # Relative import if cli.py is inside jetpwmon package

    # Or direct import if installed:
    # import jetpwmon as core_jetpwmon
except ImportError as e:
    print(f"Error: Failed to import the core 'jetpwmon' module: {e}", file=sys.stderr)
    print(
        "Ensure the package is installed correctly and the compiled module exists.",
        file=sys.stderr,
    )
    sys.exit(1)

# --- Constants ---
MAX_REFRESH_HZ = 30.0  # Max screen refresh rate
MIN_INTERVAL_SEC = 1.0 / MAX_REFRESH_HZ  # Min interval corresponding to max rate

# --- Global Flag for Signal Handling ---
keep_running = True


# --- Signal Handler ---
def signal_handler(signum, frame):
    """Sets the global flag to stop the main loop gracefully."""
    global keep_running
    # print(f"\nSignal {signum} received, stopping...") # Avoid printing directly in handler if ncurses/rich is active
    keep_running = False


# --- Argument Parsing ---
def parse_arguments():
    """Parses command line arguments."""
    parser = argparse.ArgumentParser(
        description="Dynamically monitor Jetson power consumption using jetpwmon and rich.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-f",
        "--frequency",
        type=int,
        default=50,
        metavar="HZ",
        help="Internal library sampling frequency in Hz.",
    )
    parser.add_argument(
        "-d",
        "--duration",
        type=int,
        default=0,
        metavar="SEC",
        help="Monitoring duration in seconds (0 for indefinite).",
    )
    parser.add_argument(
        "-i",
        "--interval",
        type=int,
        default=1000,
        metavar="MS",
        help=f"Screen refresh interval in milliseconds (capped at {MAX_REFRESH_HZ:.0f} Hz / ~{MIN_INTERVAL_SEC*1000:.0f} ms).",
    )
    args = parser.parse_args()

    # Validate arguments
    if args.frequency <= 0:
        parser.error("Sampling frequency must be positive.")
    if args.duration < 0:
        parser.error("Duration cannot be negative.")
    if args.interval <= 0:
        parser.error("Update interval must be positive.")

    return args


# --- Table Generation ---
def generate_table(
    monitor: PowerMonitor, start_time: float, sampling_freq: int
) -> Table:
    """Generates the rich Table object with current power data."""
    table = Table(title=None, show_header=True, header_style="bold magenta")

    # Define columns (no change needed here)
    table.add_column("Sensor Name", style="dim cyan", width=18)
    table.add_column("Power (W)", justify="right", style="green")
    table.add_column("Voltage (V)", justify="right")
    table.add_column("Current (A)", justify="right")
    table.add_column("Online", justify="center")
    table.add_column("Status", justify="left")

    try:
        # Get the latest data from the monitor - THIS RETURNS A DICT
        data_dict = monitor.get_latest_data() # Changed variable name for clarity

        # Calculate elapsed time
        elapsed_sec = time.monotonic() - start_time

        # Update table title/caption
        table.title = (
            f"Jetson Power Monitor [Sampling: {sampling_freq} Hz | "
            f"Elapsed: {elapsed_sec:.1f} s] (Press 'Ctrl+C' to quit)"
        )

        # --- FIX: Use dictionary access ---
        # Use .get() with defaults for safer access in case keys are missing
        total_data = data_dict.get('total', {})
        table.add_row(
            total_data.get('name', "Total"),
            f"{total_data.get('power', float('nan')):.2f}", # float('nan') ensures formatting doesn't fail
            f"{total_data.get('voltage', float('nan')):.2f}",
            f"{total_data.get('current', float('nan')):.2f}",
            "Yes" if total_data.get('online', False) else "No",
            total_data.get('status', "N/A"),
        )
        table.add_section()

        sensors_list = data_dict.get('sensors', [])
        sensor_count = data_dict.get('sensor_count', 0) # Though len(sensors_list) might be more direct

        if sensors_list and sensor_count > 0:
            # Iterate over the list of sensor dictionaries
            for sensor_dict in sensors_list:
                table.add_row(
                    sensor_dict.get('name', "Unknown"),
                    f"{sensor_dict.get('power', float('nan')):.2f}",
                    f"{sensor_dict.get('voltage', float('nan')):.2f}",
                    f"{sensor_dict.get('current', float('nan')):.2f}",
                    "Yes" if sensor_dict.get('online', False) else "No",
                    sensor_dict.get('status', "N/A"),
                )
        else:
            table.add_row("No individual sensor data available.", "", "", "", "", "")
        # --- END FIX ---

    except RuntimeError as e:
        # Handle errors from the C++ library during data fetch
        console = Console()
        # Avoid printing directly if inside Live context? Rich might handle this.
        # For now, add error row to table.
        table.add_row("[bold red]Error getting data[/bold red]", str(e), "", "", "", "")

    except Exception as e:
        # Handle other unexpected errors during table generation
        console = Console()
        # Avoid printing directly if inside Live context?
        table.add_row("[bold red]Unexpected error[/bold red]", str(e), "", "", "", "")


    return table
# --- Main Execution ---
def run_cli():
    """Main function for the CLI application."""
    args = parse_arguments()

    # Apply refresh rate cap
    actual_interval_sec = max(args.interval / 1000.0, MIN_INTERVAL_SEC)
    actual_refresh_rate = 1.0 / actual_interval_sec
    if args.interval / 1000.0 < MIN_INTERVAL_SEC:
        print(
            f"Note: Requested update interval faster than {MAX_REFRESH_HZ:.0f} Hz cap. "
            f"Using ~{actual_interval_sec * 1000:.0f} ms interval.",
            file=sys.stderr,
        )

    # Setup signal handling
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    monitor: Optional[PowerMonitor] = None
    sampling_started = False
    console = Console()  # For potential error messages outside Live

    try:
        # Initialize the core monitor
        console.print(f"Initializing power monitor...")
        monitor = PowerMonitor()  # RAII constructor
        console.print(f"Setting sampling frequency to {args.frequency} Hz...")
        monitor.set_sampling_frequency(args.frequency)
        monitor.reset_statistics()  # Reset stats before starting
        console.print(f"Starting background sampling...")
        monitor.start_sampling()
        sampling_started = True
        console.print("[green]Monitoring started. Display launching...[/green]")
        time.sleep(0.5)  # Short pause to allow first samples

        start_time = time.monotonic()

        # Setup the rich Live display context
        # The lambda ensures the generate_table function captures the current state
        with Live(
            generate_table(monitor, start_time, args.frequency),
            refresh_per_second=actual_refresh_rate,
            screen=True,  # Use alternate screen buffer
            transient=False,  # Keep table on screen after exit
        ) as live:

            while keep_running:
                # Check duration limit
                elapsed = time.monotonic() - start_time
                if args.duration > 0 and elapsed >= args.duration:
                    break

                # Update table content within Live context
                # This ensures the latest data is used for the next refresh cycle
                # managed by Live's refresh_per_second.
                live.update(generate_table(monitor, start_time, args.frequency))

                # Sleep briefly to prevent this loop from consuming 100% CPU
                # while waiting for the next refresh or termination signal.
                # Adjust sleep duration as needed - shorter sleep means faster
                # response to duration limit or Ctrl+C, but higher CPU usage.
                time.sleep(0.05)  # Check exit conditions roughly 20 times/sec

    except RuntimeError as e:
        # Catch init/setup errors from C++ wrapper
        console.print(f"[bold red]Fatal Error:[/bold red] {e}")
        sys.exit(1)
    except Exception as e:
        console.print(f"[bold red]Unexpected fatal error:[/bold red] {e}")
        sys.exit(1)
    finally:
        # --- Cleanup ---
        # Ensure sampling is stopped if it was started
        if monitor is not None and sampling_started:
            try:
                # print("\nStopping sampling...") # Might interfere with final screen state
                monitor.stop_sampling()
                # print("Sampling stopped.") # Might interfere
            except RuntimeError as e:
                # Use console here as Live context is finished
                console.print(f"[yellow]Warning: Error stopping sampling:[/yellow] {e}")
        # Cleanup of the C library handle is automatic via pybind11 wrapper's destructor (presumably)

        # Print final status message after Live context has exited
        print("\nPower monitoring finished.")
        # Optionally print final statistics here if needed


# Entry point for `python -m jetpwmon.cli`
if __name__ == "__main__":
    run_cli()
