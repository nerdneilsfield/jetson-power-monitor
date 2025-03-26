use jetpwmon::ffi::new_jetpwmon;

fn main() {
    let monitor = new_jetpwmon();
    
    println!("GPU 功率: {:.2} W", monitor.get_gpu_power());
    println!("GPU 温度: {:.2} °C", monitor.get_gpu_temperature());
    println!("GPU 利用率: {:.2}%", monitor.get_gpu_utilization());
    println!("GPU 内存使用率: {:.2}%", monitor.get_gpu_memory_usage());
    println!("GPU 内存带宽: {:.2} MB/s", monitor.get_gpu_memory_bandwidth());
} 