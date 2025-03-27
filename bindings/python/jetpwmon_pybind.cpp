#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "jetpwmon/jetpwmon.h"

namespace py = pybind11;

/**
 * @brief Helper function to add statistics data to a Python dictionary
 * @param d Reference to the Python dictionary
 * @param s Reference to the statistics structure
 */
void add_stats_to_dict(py::dict& d, const pm_stats_t& s) {
    d["min"] = s.min;
    d["max"] = s.max;
    d["avg"] = s.avg;
    d["total"] = s.total;
    d["count"] = s.count;
}

/**
 * @brief Wrapper class to handle C structures and provide Python interface
 */
class PowerMonitor {
public:
    /**
     * @brief Constructor that initializes the power monitor
     * @throws std::runtime_error if initialization fails
     */
    PowerMonitor() {
        pm_handle_t handle;
        if (pm_init(&handle) != PM_SUCCESS) {
            throw std::runtime_error("Failed to initialize power monitor");
        }
        handle_ = handle;
    }

    /**
     * @brief Destructor that cleans up the power monitor
     */
    ~PowerMonitor() {
        if (handle_) {
            pm_cleanup(handle_);
        }
    }

    /**
     * @brief Set the sampling frequency
     * @param frequency_hz Sampling frequency in Hz
     * @throws std::runtime_error if setting frequency fails
     */
    void set_sampling_frequency(int frequency_hz) {
        if (pm_set_sampling_frequency(handle_, frequency_hz) != PM_SUCCESS) {
            throw std::runtime_error("Failed to set sampling frequency");
        }
    }

    /**
     * @brief Get the current sampling frequency
     * @return Current sampling frequency in Hz
     * @throws std::runtime_error if getting frequency fails
     */
    int get_sampling_frequency() {
        int frequency;
        if (pm_get_sampling_frequency(handle_, &frequency) != PM_SUCCESS) {
            throw std::runtime_error("Failed to get sampling frequency");
        }
        return frequency;
    }

    /**
     * @brief Start power sampling
     * @throws std::runtime_error if starting sampling fails
     */
    void start_sampling() {
        if (pm_start_sampling(handle_) != PM_SUCCESS) {
            throw std::runtime_error("Failed to start sampling");
        }
    }

    /**
     * @brief Stop power sampling
     * @throws std::runtime_error if stopping sampling fails
     */
    void stop_sampling() {
        if (pm_stop_sampling(handle_) != PM_SUCCESS) {
            throw std::runtime_error("Failed to stop sampling");
        }
    }

    /**
     * @brief Check if sampling is active
     * @return True if sampling is active, false otherwise
     * @throws std::runtime_error if checking status fails
     */
    bool is_sampling() {
        bool sampling;
        if (pm_is_sampling(handle_, &sampling) != PM_SUCCESS) {
            throw std::runtime_error("Failed to check sampling status");
        }
        return sampling;
    }

    /**
     * @brief Get the latest power data
     * @return Python dictionary containing the latest power data
     * @throws std::runtime_error if getting data fails
     */
    py::object get_latest_data() {
        pm_power_data_t data;
        if (pm_get_latest_data(handle_, &data) != PM_SUCCESS) {
            throw std::runtime_error("Failed to get latest data");
        }

        py::dict result;
        py::dict total;
        total["name"] = std::string(data.total.name);
        total["type"] = data.total.type;
        total["voltage"] = data.total.voltage;
        total["current"] = data.total.current;
        total["power"] = data.total.power;
        total["online"] = data.total.online;
        total["status"] = std::string(data.total.status);
        total["warning_threshold"] = data.total.warning_threshold;
        total["critical_threshold"] = data.total.critical_threshold;
        result["total"] = total;

        py::list sensors;
        if (data.sensor_count > 0 && data.sensors != nullptr) {
            for (int i = 0; i < data.sensor_count; i++) {
                py::dict sensor;
                sensor["name"] = std::string(data.sensors[i].name);
                sensor["type"] = data.sensors[i].type;
                sensor["voltage"] = data.sensors[i].voltage;
                sensor["current"] = data.sensors[i].current;
                sensor["power"] = data.sensors[i].power;
                sensor["online"] = data.sensors[i].online;
                sensor["status"] = std::string(data.sensors[i].status);
                sensor["warning_threshold"] = data.sensors[i].warning_threshold;
                sensor["critical_threshold"] = data.sensors[i].critical_threshold;
                sensors.append(sensor);
            }
        }
        result["sensors"] = sensors;
        result["sensor_count"] = data.sensor_count;

        return result;
    }

    /**
     * @brief Get power statistics
     * @return Python dictionary containing power statistics
     * @throws std::runtime_error if getting statistics fails
     */
    py::object get_statistics() {
        pm_power_stats_t stats;
        if (pm_get_statistics(handle_, &stats) != PM_SUCCESS) {
            throw std::runtime_error("Failed to get statistics");
        }

        py::dict result;
        py::dict total;
        total["name"] = std::string(stats.total.name);
        py::dict voltage_stats;
        py::dict current_stats;
        py::dict power_stats;

        add_stats_to_dict(voltage_stats, stats.total.voltage);
        add_stats_to_dict(current_stats, stats.total.current);
        add_stats_to_dict(power_stats, stats.total.power);

        total["voltage"] = voltage_stats;
        total["current"] = current_stats;
        total["power"] = power_stats;
        result["total"] = total;

        py::list sensors;
        if (stats.sensor_count > 0 && stats.sensors != nullptr) {
            for (int i = 0; i < stats.sensor_count; i++) {
                py::dict sensor;
                sensor["name"] = std::string(stats.sensors[i].name);
                
                py::dict sensor_voltage_stats;
                py::dict sensor_current_stats;
                py::dict sensor_power_stats;

                add_stats_to_dict(sensor_voltage_stats, stats.sensors[i].voltage);
                add_stats_to_dict(sensor_current_stats, stats.sensors[i].current);
                add_stats_to_dict(sensor_power_stats, stats.sensors[i].power);

                sensor["voltage"] = sensor_voltage_stats;
                sensor["current"] = sensor_current_stats;
                sensor["power"] = sensor_power_stats;

                sensors.append(sensor);
            }
        }
        result["sensors"] = sensors;
        result["sensor_count"] = stats.sensor_count;

        return result;
    }

    /**
     * @brief Reset power statistics
     * @throws std::runtime_error if resetting statistics fails
     */
    void reset_statistics() {
        if (pm_reset_statistics(handle_) != PM_SUCCESS) {
            throw std::runtime_error("Failed to reset statistics");
        }
    }

    /**
     * @brief Get the number of sensors
     * @return Number of sensors
     * @throws std::runtime_error if getting sensor count fails
     */
    int get_sensor_count() {
        int count;
        if (pm_get_sensor_count(handle_, &count) != PM_SUCCESS) {
            throw std::runtime_error("Failed to get sensor count");
        }
        return count;
    }

    /**
     * @brief Get the names of all sensors
     * @return Python list containing sensor names
     * @throws std::runtime_error if getting sensor names fails
     * @deprecated This function is unsafe and will be removed in a future version.
     *             Please use get_latest_data() or get_statistics() instead.
     */
    py::object get_sensor_names() {
        int count = get_sensor_count();
        std::vector<char*> names(count);
        for (int i = 0; i < count; i++) {
            names[i] = new char[64];  // 使用固定大小的缓冲区，与 pm_sensor_data_t 中的 name 字段大小一致
        }

        if (pm_get_sensor_names(handle_, names.data(), &count) != PM_SUCCESS) {
            for (int i = 0; i < count; i++) {
                delete[] names[i];
            }
            throw std::runtime_error("Failed to get sensor names");
        }

        py::list result;
        for (int i = 0; i < count; i++) {
            result.append(std::string(names[i]));
            delete[] names[i];
        }

        return result;
    }

private:
    pm_handle_t handle_; ///< Handle to the power monitor instance
};

PYBIND11_MODULE(_core, m) {
    m.doc() = "Python bindings for Jetson Power Monitor";

    py::class_<PowerMonitor>(m, "PowerMonitor")
        .def(py::init<>())
        .def("set_sampling_frequency", &PowerMonitor::set_sampling_frequency)
        .def("get_sampling_frequency", &PowerMonitor::get_sampling_frequency)
        .def("start_sampling", &PowerMonitor::start_sampling)
        .def("stop_sampling", &PowerMonitor::stop_sampling)
        .def("is_sampling", &PowerMonitor::is_sampling)
        .def("get_latest_data", &PowerMonitor::get_latest_data)
        .def("get_statistics", &PowerMonitor::get_statistics)
        .def("reset_statistics", &PowerMonitor::reset_statistics)
        .def("get_sensor_count", &PowerMonitor::get_sensor_count)
        .def("get_sensor_names", [](PowerMonitor& self) {
            PyErr_WarnEx(PyExc_DeprecationWarning,
                        "This function is unsafe and will be removed in a future version. "
                        "Please use get_latest_data() or get_statistics() instead.",
                        1);
            return self.get_sensor_names();
        });

    // 导出错误码枚举
    py::enum_<pm_error_t>(m, "ErrorCode")
        .value("SUCCESS", PM_SUCCESS)
        .value("ERROR_INIT_FAILED", PM_ERROR_INIT_FAILED)
        .value("ERROR_NOT_INITIALIZED", PM_ERROR_NOT_INITIALIZED)
        .value("ERROR_ALREADY_RUNNING", PM_ERROR_ALREADY_RUNNING)
        .value("ERROR_NOT_RUNNING", PM_ERROR_NOT_RUNNING)
        .value("ERROR_INVALID_FREQUENCY", PM_ERROR_INVALID_FREQUENCY)
        .value("ERROR_NO_SENSORS", PM_ERROR_NO_SENSORS)
        .value("ERROR_FILE_ACCESS", PM_ERROR_FILE_ACCESS)
        .value("ERROR_MEMORY", PM_ERROR_MEMORY)
        .value("ERROR_THREAD", PM_ERROR_THREAD)
        .export_values();

    // 导出传感器类型枚举
    py::enum_<pm_sensor_type_t>(m, "SensorType")
        .value("UNKNOWN", PM_SENSOR_TYPE_UNKNOWN)
        .value("I2C", PM_SENSOR_TYPE_I2C)
        .value("SYSTEM", PM_SENSOR_TYPE_SYSTEM)
        .export_values();

    // 导出错误字符串函数
    m.def("error_string", &pm_error_string);
} 