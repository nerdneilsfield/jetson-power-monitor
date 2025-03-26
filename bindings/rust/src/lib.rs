use std::ffi::{c_void, CString};
use std::ptr::NonNull;

/// A handle to the power monitor instance
#[repr(C)]
pub struct PowerHandle(*mut c_void);

/// Types of power sensors supported by the library
#[repr(C)]
pub enum SensorType {
    /// Unknown sensor type
    Unknown = 0,
    /// I2C power sensor (e.g., INA3221)
    I2C = 1,
    /// System power supply
    System = 2,
}

/// Power data for a single sensor
#[repr(C)]
pub struct SensorData {
    /// Sensor name
    pub name: [u8; 64],
    /// Sensor type
    pub type_: SensorType,
    /// Voltage in volts
    pub voltage: f64,
    /// Current in amperes
    pub current: f64,
    /// Power in watts
    pub power: f64,
    /// Whether the sensor is online
    pub online: bool,
    /// Status string (if available)
    pub status: [u8; 32],
    /// Warning threshold in watts
    pub warning_threshold: f64,
    /// Critical threshold in watts
    pub critical_threshold: f64,
}

/// Statistical data for a metric
#[repr(C)]
pub struct Stats {
    /// Minimum value
    pub min: f64,
    /// Maximum value
    pub max: f64,
    /// Average value
    pub avg: f64,
    /// Sum of all samples
    pub total: f64,
    /// Number of samples
    pub count: u64,
}

/// Power statistics for a sensor
#[repr(C)]
pub struct SensorStats {
    /// Sensor name
    pub name: [u8; 64],
    /// Voltage statistics
    pub voltage: Stats,
    /// Current statistics
    pub current: Stats,
    /// Power statistics
    pub power: Stats,
}

/// Overall power data
#[repr(C)]
pub struct PowerData {
    /// Total power consumption
    pub total: SensorData,
    /// Array of sensor data
    pub sensors: *mut SensorData,
    /// Number of sensors
    pub sensor_count: i32,
}

/// Overall power statistics
#[repr(C)]
pub struct PowerStats {
    /// Total power statistics
    pub total: SensorStats,
    /// Array of sensor statistics
    pub sensors: *mut SensorStats,
    /// Number of sensors
    pub sensor_count: i32,
}

/// Error codes returned by library functions
#[derive(Debug)]
pub enum Error {
    /// Initialization failed
    InitFailed,
    /// Library not initialized
    NotInitialized,
    /// Sampling already running
    AlreadyRunning,
    /// Sampling not running
    NotRunning,
    /// Invalid sampling frequency
    InvalidFrequency,
    /// No power sensors found
    NoSensors,
    /// Error accessing sensor files
    FileAccess,
    /// Memory allocation error
    Memory,
    /// Thread creation/management error
    Thread,
    /// Unknown error code
    Unknown(i32),
}

impl From<i32> for Error {
    fn from(code: i32) -> Self {
        match code {
            -1 => Error::InitFailed,
            -2 => Error::NotInitialized,
            -3 => Error::AlreadyRunning,
            -4 => Error::NotRunning,
            -5 => Error::InvalidFrequency,
            -6 => Error::NoSensors,
            -7 => Error::FileAccess,
            -8 => Error::Memory,
            -9 => Error::Thread,
            _ => Error::Unknown(code),
        }
    }
}

/// A power monitor instance that provides functionality to monitor power consumption
/// from various sources (I2C sensors, system power supplies), collect statistics,
/// and control the sampling process.
pub struct PowerMonitor {
    handle: NonNull<c_void>,
}

impl PowerMonitor {
    /// Creates a new power monitor instance
    /// 
    /// This function discovers power sensors on the system and initializes
    /// the power monitor library.
    /// 
    /// # Returns
    /// 
    /// * `Ok(PowerMonitor)` - A new power monitor instance
    /// * `Err(Error)` - An error code if initialization fails
    pub fn new() -> Result<Self, Error> {
        let mut handle = std::ptr::null_mut();
        let result = unsafe { pm_init(&mut handle) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(Self {
            handle: NonNull::new(handle).unwrap(),
        })
    }

    /// Sets the sampling frequency
    /// 
    /// # Arguments
    /// 
    /// * `frequency_hz` - Sampling frequency in Hz (must be > 0)
    /// 
    /// # Returns
    /// 
    /// * `Ok(())` - Success
    /// * `Err(Error)` - An error code if setting frequency fails
    pub fn set_sampling_frequency(&self, frequency_hz: i32) -> Result<(), Error> {
        let result = unsafe { pm_set_sampling_frequency(self.handle.as_ptr(), frequency_hz) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(())
    }

    /// Gets the current sampling frequency
    /// 
    /// # Returns
    /// 
    /// * `Ok(i32)` - Current sampling frequency in Hz
    /// * `Err(Error)` - An error code if getting frequency fails
    pub fn get_sampling_frequency(&self) -> Result<i32, Error> {
        let mut frequency = 0;
        let result = unsafe { pm_get_sampling_frequency(self.handle.as_ptr(), &mut frequency) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(frequency)
    }

    /// Starts sampling
    /// 
    /// This function starts the sampling thread that periodically reads
    /// power data from all discovered sensors.
    /// 
    /// # Returns
    /// 
    /// * `Ok(())` - Success
    /// * `Err(Error)` - An error code if starting sampling fails
    pub fn start_sampling(&self) -> Result<(), Error> {
        let result = unsafe { pm_start_sampling(self.handle.as_ptr()) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(())
    }

    /// Stops sampling
    /// 
    /// This function stops the sampling thread.
    /// 
    /// # Returns
    /// 
    /// * `Ok(())` - Success
    /// * `Err(Error)` - An error code if stopping sampling fails
    pub fn stop_sampling(&self) -> Result<(), Error> {
        let result = unsafe { pm_stop_sampling(self.handle.as_ptr()) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(())
    }

    /// Checks if sampling is active
    /// 
    /// # Returns
    /// 
    /// * `Ok(bool)` - true if sampling is active, false otherwise
    /// * `Err(Error)` - An error code if checking status fails
    pub fn is_sampling(&self) -> Result<bool, Error> {
        let mut is_sampling = false;
        let result = unsafe { pm_is_sampling(self.handle.as_ptr(), &mut is_sampling) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(is_sampling)
    }

    /// Gets the latest power data
    /// 
    /// # Returns
    /// 
    /// * `Ok(PowerData)` - Latest power data
    /// * `Err(Error)` - An error code if getting data fails
    pub fn get_latest_data(&self) -> Result<PowerData, Error> {
        let mut data = PowerData {
            total: unsafe { std::mem::zeroed() },
            sensors: std::ptr::null_mut(),
            sensor_count: 0,
        };
        let result = unsafe { pm_get_latest_data(self.handle.as_ptr(), &mut data) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(data)
    }

    /// Gets the power statistics
    /// 
    /// # Returns
    /// 
    /// * `Ok(PowerStats)` - Power statistics
    /// * `Err(Error)` - An error code if getting statistics fails
    pub fn get_statistics(&self) -> Result<PowerStats, Error> {
        let mut stats = PowerStats {
            total: unsafe { std::mem::zeroed() },
            sensors: std::ptr::null_mut(),
            sensor_count: 0,
        };
        let result = unsafe { pm_get_statistics(self.handle.as_ptr(), &mut stats) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(stats)
    }

    /// Resets the statistics
    /// 
    /// This function resets all collected statistics.
    /// 
    /// # Returns
    /// 
    /// * `Ok(())` - Success
    /// * `Err(Error)` - An error code if resetting statistics fails
    pub fn reset_statistics(&self) -> Result<(), Error> {
        let result = unsafe { pm_reset_statistics(self.handle.as_ptr()) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(())
    }

    /// Gets the number of sensors
    /// 
    /// # Returns
    /// 
    /// * `Ok(i32)` - Number of sensors
    /// * `Err(Error)` - An error code if getting sensor count fails
    pub fn get_sensor_count(&self) -> Result<i32, Error> {
        let mut count = 0;
        let result = unsafe { pm_get_sensor_count(self.handle.as_ptr(), &mut count) };
        if result != 0 {
            return Err(result.into());
        }
        Ok(count)
    }

    /// Gets the sensor names
    /// 
    /// This function returns a vector of strings containing the names of all sensors.
    /// 
    /// # Returns
    /// 
    /// * `Ok(Vec<String>)` - Vector of sensor names
    /// * `Err(Error)` - An error code if getting sensor names fails
    pub fn get_sensor_names(&self) -> Result<Vec<String>, Error> {
        let count = self.get_sensor_count()?;
        let mut names = vec![std::ptr::null_mut(); count as usize];
        let mut count = count;
        let result = unsafe {
            pm_get_sensor_names(
                self.handle.as_ptr(),
                names.as_mut_ptr(),
                &mut count,
            )
        };
        if result != 0 {
            return Err(result.into());
        }
        Ok(names
            .into_iter()
            .map(|ptr| unsafe { CString::from_raw(ptr).into_string().unwrap() })
            .collect())
    }
}

impl Drop for PowerMonitor {
    /// Cleans up resources when the power monitor is dropped
    /// 
    /// This function stops any active sampling and frees all resources
    /// allocated by the library.
    fn drop(&mut self) {
        unsafe {
            pm_cleanup(self.handle.as_ptr());
        }
    }
}

extern "C" {
    fn pm_init(handle: *mut *mut c_void) -> i32;
    fn pm_cleanup(handle: *mut c_void) -> i32;
    fn pm_set_sampling_frequency(handle: *mut c_void, frequency_hz: i32) -> i32;
    fn pm_get_sampling_frequency(handle: *mut c_void, frequency_hz: *mut i32) -> i32;
    fn pm_start_sampling(handle: *mut c_void) -> i32;
    fn pm_stop_sampling(handle: *mut c_void) -> i32;
    fn pm_is_sampling(handle: *mut c_void, is_sampling: *mut bool) -> i32;
    fn pm_get_latest_data(handle: *mut c_void, data: *mut PowerData) -> i32;
    fn pm_get_statistics(handle: *mut c_void, stats: *mut PowerStats) -> i32;
    fn pm_reset_statistics(handle: *mut c_void) -> i32;
    fn pm_get_sensor_count(handle: *mut c_void, count: *mut i32) -> i32;
    fn pm_get_sensor_names(handle: *mut c_void, names: *mut *mut i8, count: *mut i32) -> i32;
}