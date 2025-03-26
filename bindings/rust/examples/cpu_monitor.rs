use jetpwmon::PowerMonitor;
use std::thread;
use std::time::Duration;

fn cpu_intensive_task() {
    println!("开始执行CPU密集型任务...");
    
    // 模拟CPU密集型任务
    let size = 2000;
    let mut matrix1 = vec![0.0; size * size];
    let mut matrix2 = vec![0.0; size * size];
    let mut result = vec![0.0; size * size];
    
    // 初始化矩阵
    for i in 0..size * size {
        matrix1[i] = rand::random();
        matrix2[i] = rand::random();
    }
    
    // 执行矩阵乘法
    for i in 0..size {
        for j in 0..size {
            result[i * size + j] = 0.0;
            for k in 0..size {
                result[i * size + j] += matrix1[i * size + k] * matrix2[k * size + j];
            }
        }
    }
    
    println!("CPU密集型任务完成");
}

fn monitor_power_consumption<F>(task_func: F) -> Result<(), jetpwmon::Error>
where
    F: FnOnce(),
{
    // 初始化功耗监控器
    let monitor = PowerMonitor::new()?;
    
    // 设置采样频率为1000Hz
    monitor.set_sampling_frequency(1000)?;
    
    // 重置统计数据
    monitor.reset_statistics()?;
    
    // 开始采样
    println!("开始功耗采样...");
    monitor.start_sampling()?;
    
    // 执行任务
    task_func();
    
    // 等待一小段时间确保数据采集完整
    thread::sleep(Duration::from_micros(500000));
    
    // 停止采样
    monitor.stop_sampling()?;
    
    // 获取统计数据
    let stats = monitor.get_statistics()?;
    
    // 打印总功耗统计信息
    println!("\n功耗统计信息:");
    println!("总功耗:");
    println!("  最小值: {:.2} W", stats.total.power.min);
    println!("  最大值: {:.2} W", stats.total.power.max);
    println!("  平均值: {:.2} W", stats.total.power.avg);
    println!("  总能耗: {:.2} J", stats.total.power.total);
    println!("  采样次数: {}", stats.total.power.count);
    
    // 打印各个传感器的功耗信息
    println!("\n各传感器功耗信息:");
    let sensor_count = monitor.get_sensor_count()?;
    for i in 0..sensor_count {
        let sensor = unsafe { &*stats.sensors.add(i as usize) };
        let name = String::from_utf8_lossy(&sensor.name).trim_matches('\0').to_string();
        println!("\n传感器: {}", name);
        println!("  最小值: {:.2} W", sensor.power.min);
        println!("  最大值: {:.2} W", sensor.power.max);
        println!("  平均值: {:.2} W", sensor.power.avg);
        println!("  总能耗: {:.2} J", sensor.power.total);
        println!("  采样次数: {}", sensor.power.count);
    }
    
    Ok(())
}

fn main() {
    println!("Jetson Power Monitor Rust 示例程序");
    println!("=================================");
    
    // 监控CPU密集型任务的功耗
    if let Err(e) = monitor_power_consumption(cpu_intensive_task) {
        eprintln!("错误: {:?}", e);
    }
} 