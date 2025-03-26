use jetpwmon::PowerMonitor;

fn main() {
    let monitor = PowerMonitor::new().unwrap();
    
    // 设置采样频率为1Hz
    monitor.set_sampling_frequency(1).unwrap();
    
    // 开始采样
    monitor.start_sampling().unwrap();
    
    // 等待一段时间收集数据
    std::thread::sleep(std::time::Duration::from_secs(1));
    
    // 获取最新数据
    let data = monitor.get_latest_data().unwrap();
    
    // 打印总功耗信息
    println!("总功耗: {:.2} W", data.total.power);
    println!("总电压: {:.2} V", data.total.voltage);
    println!("总电流: {:.2} A", data.total.current);
    println!("状态: {}", String::from_utf8_lossy(&data.total.status).trim_matches('\0'));
    
    // 打印各个传感器的信息
    println!("\n各传感器信息:");
    for i in 0..data.sensor_count {
        let sensor = unsafe { &*data.sensors.add(i as usize) };
        let name = String::from_utf8_lossy(&sensor.name).trim_matches('\0').to_string();
        println!("\n传感器: {}", name);
        println!("  类型: {:?}", sensor.type_);
        println!("  功率: {:.2} W", sensor.power);
        println!("  电压: {:.2} V", sensor.voltage);
        println!("  电流: {:.2} A", sensor.current);
        println!("  状态: {}", String::from_utf8_lossy(&sensor.status).trim_matches('\0'));
        println!("  警告阈值: {:.2} W", sensor.warning_threshold);
        println!("  临界阈值: {:.2} W", sensor.critical_threshold);
    }
    
    // 停止采样
    monitor.stop_sampling().unwrap();
} 