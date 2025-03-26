use jetpwmon::{PowerMonitor, Error, SensorType};
use std::thread;
use std::time::Duration;

/// Test initialization of PowerMonitor
#[test]
fn test_init() {
    let monitor = PowerMonitor::new().unwrap();
    assert!(monitor.get_sensor_count().unwrap() >= 0);
}

/// Test setting and getting sampling frequency
#[test]
fn test_sampling_frequency() {
    let monitor = PowerMonitor::new().unwrap();
    
    // Test setting sampling frequency
    let test_freq = 10;
    monitor.set_sampling_frequency(test_freq).unwrap();
    
    // Test getting sampling frequency
    let actual_freq = monitor.get_sampling_frequency().unwrap();
    assert_eq!(actual_freq, test_freq);
    
    // Test invalid sampling frequency
    assert!(matches!(
        monitor.set_sampling_frequency(0).unwrap_err(),
        Error::InvalidFrequency
    ));
}

/// Test starting and stopping sampling
#[test]
fn test_sampling_control() {
    let monitor = PowerMonitor::new().unwrap();
    
    // Test starting sampling
    monitor.start_sampling().unwrap();
    assert!(monitor.is_sampling().unwrap());
    
    // Test duplicate start sampling
    assert!(matches!(
        monitor.start_sampling().unwrap_err(),
        Error::AlreadyRunning
    ));
    
    // Test stopping sampling
    monitor.stop_sampling().unwrap();
    assert!(!monitor.is_sampling().unwrap());
    
    // Test duplicate stop sampling
    assert!(matches!(
        monitor.stop_sampling().unwrap_err(),
        Error::NotRunning
    ));
}

/// Test data collection functionality
#[test]
fn test_data_collection() {
    let monitor = PowerMonitor::new().unwrap();
    
    // Set sampling frequency and start sampling
    monitor.set_sampling_frequency(10).unwrap();
    monitor.start_sampling().unwrap();
    
    // Wait for data collection
    thread::sleep(Duration::from_millis(500));
    
    // Get latest data
    let data = monitor.get_latest_data().unwrap();
    
    // Verify total data
    assert!(data.total.power >= 0.0);
    assert!(data.total.current >= 0.0);
    assert!(data.total.voltage >= 0.0);
    assert!(data.total.online);
    
    // Verify sensor data
    let sensor_count = monitor.get_sensor_count().unwrap();
    if sensor_count > 0 {
        let sensor = unsafe { &*data.sensors };
        assert!(!String::from_utf8_lossy(&sensor.name).trim_matches('\0').is_empty());
        assert!(sensor.power >= 0.0);
        assert!(sensor.current >= 0.0);
        assert!(sensor.voltage >= 0.0);
        assert!(sensor.online);
        assert!(sensor.warning_threshold >= 0.0);
        assert!(sensor.critical_threshold >= 0.0);
    }
    
    monitor.stop_sampling().unwrap();
}

/// Test statistics collection
#[test]
fn test_statistics() {
    let monitor = PowerMonitor::new().unwrap();
    
    // Reset statistics
    monitor.reset_statistics().unwrap();
    
    // Set sampling frequency and start sampling
    monitor.set_sampling_frequency(10).unwrap();
    monitor.start_sampling().unwrap();
    
    // Wait for data collection
    thread::sleep(Duration::from_millis(500));
    
    // Get statistics
    let stats = monitor.get_statistics().unwrap();
    
    // Verify total statistics
    assert!(stats.total.power.min >= 0.0);
    assert!(stats.total.power.max >= 0.0);
    assert!(stats.total.power.avg >= 0.0);
    assert!(stats.total.power.total >= 0.0);
    assert!(stats.total.power.count > 0);
    
    // Verify sensor statistics
    let sensor_count = monitor.get_sensor_count().unwrap();
    if sensor_count > 0 {
        let sensor = unsafe { &*stats.sensors };
        assert!(!String::from_utf8_lossy(&sensor.name).trim_matches('\0').is_empty());
        assert!(sensor.power.min >= 0.0);
        assert!(sensor.power.max >= 0.0);
        assert!(sensor.power.avg >= 0.0);
        assert!(sensor.power.total >= 0.0);
        assert!(sensor.power.count > 0);
    }
    
    monitor.stop_sampling().unwrap();
}

/// Test sensor information retrieval
#[test]
fn test_sensor_info() {
    let monitor = PowerMonitor::new().unwrap();
    
    // Get sensor count
    let count = monitor.get_sensor_count().unwrap();
    assert!(count >= 0);
    
    // Get sensor names
    let names = monitor.get_sensor_names().unwrap();
    assert_eq!(names.len(), count as usize);
    for name in names {
        assert!(!name.is_empty());
    }
}

/// Test error code values
#[test]
fn test_error_handling() {
    assert_eq!(Error::InitFailed as i32, -1);
    assert_eq!(Error::NotInitialized as i32, -2);
    assert_eq!(Error::AlreadyRunning as i32, -3);
    assert_eq!(Error::NotRunning as i32, -4);
    assert_eq!(Error::InvalidFrequency as i32, -5);
    assert_eq!(Error::NoSensors as i32, -6);
    assert_eq!(Error::FileAccess as i32, -7);
    assert_eq!(Error::Memory as i32, -8);
    assert_eq!(Error::Thread as i32, -9);
}

/// Test sensor type values
#[test]
fn test_sensor_types() {
    assert_eq!(SensorType::Unknown as u32, 0);
    assert_eq!(SensorType::I2C as u32, 1);
    assert_eq!(SensorType::System as u32, 2);
}