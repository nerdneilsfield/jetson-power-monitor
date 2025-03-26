use jetpwmon::PowerMonitor;
use std::thread;
use std::time::Duration;
use std::sync::Arc;
use rand::Rng;

const MATRIX_SIZE: usize = 5000;
const NUM_THREADS: usize = 4;
const NUM_ITERATIONS: usize = 10;

fn matrix_multiply_thread(thread_id: usize) {
    println!("线程 {} 开始执行...", thread_id);

    // 使用 ndarray 进行矩阵乘法
    #[cfg(test)]
    {
        use ndarray::{Array2, s};
        use ndarray::linalg::Dot;

        // 创建随机矩阵
        let mut rng = rand::thread_rng();
        let mut a = Array2::zeros((MATRIX_SIZE, MATRIX_SIZE));
        let mut b = Array2::zeros((MATRIX_SIZE, MATRIX_SIZE));
        
        // 填充随机值
        for i in 0..MATRIX_SIZE {
            for j in 0..MATRIX_SIZE {
                a[[i, j]] = rng.gen_range(-1.0..1.0);
                b[[i, j]] = rng.gen_range(-1.0..1.0);
            }
        }

        // 执行多次矩阵乘法
        let mut c = Array2::zeros((MATRIX_SIZE, MATRIX_SIZE));
        for _ in 0..NUM_ITERATIONS {
            c = a.dot(&b);
            a = c.clone();
        }
    }

    println!("线程 {} 完成执行", thread_id);
}

fn main() {
    println!("Jetson Power Monitor - 矩阵乘法测试");
    println!("=================================");

    // 初始化功耗监控器
    let monitor = PowerMonitor::new().unwrap();
    
    // 设置采样频率为1000Hz
    monitor.set_sampling_frequency(1000).unwrap();
    
    // 重置统计数据
    monitor.reset_statistics().unwrap();
    
    // 开始采样
    println!("开始功耗采样...");
    monitor.start_sampling().unwrap();
    
    // 记录开始时间
    let start_time = std::time::Instant::now();
    
    // 创建并启动线程
    let mut handles = vec![];
    for i in 0..NUM_THREADS {
        let handle = thread::spawn(move || {
            matrix_multiply_thread(i);
        });
        handles.push(handle);
    }
    
    // 等待所有线程完成
    for handle in handles {
        handle.join().unwrap();
    }
    
    // 计算总执行时间
    let total_time = start_time.elapsed().as_secs_f64();
    println!("\n总执行时间: {:.2} 秒", total_time);
    
    // 等待一小段时间确保数据采集完整
    thread::sleep(Duration::from_micros(500000));
    
    // 停止采样
    monitor.stop_sampling().unwrap();
    
    // 获取统计数据
    let stats = monitor.get_statistics().unwrap();
    
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
    for i in 0..stats.sensor_count {
        let sensor = unsafe { &*stats.sensors.add(i as usize) };
        let name = String::from_utf8_lossy(&sensor.name).trim_matches('\0').to_string();
        println!("\n传感器: {}", name);
        println!("  最小值: {:.2} W", sensor.power.min);
        println!("  最大值: {:.2} W", sensor.power.max);
        println!("  平均值: {:.2} W", sensor.power.avg);
        println!("  总能耗: {:.2} J", sensor.power.total);
        println!("  采样次数: {}", sensor.power.count);
    }
} 