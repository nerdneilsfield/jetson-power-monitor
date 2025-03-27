fn main() {
    let target = std::env::var("TARGET").unwrap_or_else(|_| String::from(""));
    let host = std::env::var("HOST").unwrap_or_else(|_| String::from(""));
    
    let mut build = cc::Build::new();
    
    // 基本配置
    build.file("vendor/src/jetpwmon.c")
        .include("vendor/include")
        .flag("-std=c99")
        .flag("-Wall")
        .flag("-Wextra");
    
    // 如果是交叉编译到 ARM64
    if target.contains("aarch64") && !host.contains("aarch64") {
        build.compiler("aarch64-linux-gnu-gcc");
        println!("cargo:rustc-link-search=native=/usr/aarch64-linux-gnu/lib");
        println!("cargo:rustc-link-lib=ncurses");
    } else {
        println!("cargo:rustc-link-lib=ncurses");
    }
    
    build.static_flag(true)
         .compile("jetpwmon");

    println!("cargo:rustc-link-search=native={}", std::env::var("OUT_DIR").unwrap());
    println!("cargo:rustc-link-lib=static=jetpwmon");
    println!("cargo:rustc-link-lib=pthread");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=vendor/src/jetpwmon.c");
    println!("cargo:rerun-if-changed=vendor/include/jetpwmon.h");
} 