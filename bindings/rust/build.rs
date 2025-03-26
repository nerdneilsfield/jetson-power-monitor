fn main() {
    // 编译 C 库
    cc::Build::new()
        .file("../../src/jetpwmon.c")
        .include("../../include")
        .flag("-std=c99")
        .compile("jetpwmon");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=../../src/jetpwmon.c");
    println!("cargo:rerun-if-changed=../../include/jetpwmon.h");
} 