use jetpwmon::PowerMonitor;
use std::thread;
use std::time::Duration;
use ndarray::{Array2, s};
use ndarray::linalg::Dot;
use rand::Rng;

/// Matrix size for multiplication
const MATRIX_SIZE: usize = 1000;
/// Number of threads to use for matrix multiplication
const NUM_THREADS: usize = 4;
/// Number of iterations for matrix multiplication
const NUM_ITERATIONS: usize = 10;

/// Executes matrix multiplication in a thread
fn matrix_multiply_thread(thread_id: usize) {
    println!("Thread {} starting execution...", thread_id);

    // Using ndarray for matrix multiplication
    let mut rng = rand::thread_rng();
    let mut a = Array2::zeros((MATRIX_SIZE, MATRIX_SIZE));
    let mut b = Array2::zeros((MATRIX_SIZE, MATRIX_SIZE));
    
    // Filling with random values
    for i in 0..MATRIX_SIZE {
        for j in 0..MATRIX_SIZE {
            a[[i, j]] = rng.gen_range(-1.0..1.0);
            b[[i, j]] = rng.gen_range(-1.0..1.0);
        }
    }

    // Performing multiple matrix multiplications
    let mut c = Array2::zeros((MATRIX_SIZE, MATRIX_SIZE));
    for _ in 0..NUM_ITERATIONS {
        c = a.dot(&b);
        a = c.clone();
    }

    println!("Thread {} completed execution", thread_id);
}

fn main() {
    println!("Jetson Power Monitor - Matrix Multiplication Test");
    println!("=================================");

    // Initializing power monitor
    let monitor = PowerMonitor::new().unwrap();
    
    // Setting sampling frequency to 1000Hz
    monitor.set_sampling_frequency(1000).unwrap();
    
    // Resetting statistics
    monitor.reset_statistics().unwrap();
    
    // Starting sampling
    println!("Starting power sampling...");
    monitor.start_sampling().unwrap();
    
    // Recording start time
    let start_time = std::time::Instant::now();
    
    // Creating and starting threads
    let mut handles = vec![];
    for i in 0..NUM_THREADS {
        let handle = thread::spawn(move || {
            matrix_multiply_thread(i);
        });
        handles.push(handle);
    }
    
    // Waiting for all threads to complete
    for handle in handles {
        handle.join().unwrap();
    }
    
    // Calculating total execution time
    let total_time = start_time.elapsed().as_secs_f64();
    println!("\nTotal execution time: {:.2} seconds", total_time);
    
    // Waiting a short period to ensure data collection is complete
    thread::sleep(Duration::from_micros(500000));
    
    // Stopping sampling
    monitor.stop_sampling().unwrap();
    
    // Getting statistics
    let stats = monitor.get_statistics().unwrap();
    
    // Printing total power consumption statistics
    println!("\nPower Consumption Statistics:");
    println!("Total Power Consumption:");
    println!("  Minimum Value: {:.2} W", stats.total.power.min);
    println!("  Maximum Value: {:.2} W", stats.total.power.max);
    println!("  Average Value: {:.2} W", stats.total.power.avg);
    println!("  Total Energy Consumption: {:.2} J", stats.total.power.total);
    println!("  Sample Count: {}", stats.total.power.count);
    
    // Printing power consumption information for each sensor
    println!("\nPower Consumption Information for Each Sensor:");
    for i in 0..stats.sensor_count {
        let sensor = unsafe { &*stats.sensors.add(i as usize) };
        let name = String::from_utf8_lossy(&sensor.name).trim_matches('\0').to_string();
        println!("\nSensor: {}", name);
        println!("  Minimum Value: {:.2} W", sensor.power.min);
        println!("  Maximum Value: {:.2} W", sensor.power.max);
        println!("  Average Value: {:.2} W", sensor.power.avg);
        println!("  Total Energy Consumption: {:.2} J", sensor.power.total);
        println!("  Sample Count: {}", sensor.power.count);
    }
} 