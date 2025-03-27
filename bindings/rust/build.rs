fn main() {
    // 编译 C 库
    cc::Build::new()
        .file("vendor/src/jetpwmon.c")
        .include("vendor/include")
        .flag("-std=c99")
        .flag("-Wall")
        .flag("-Wextra")
        .static_flag(true)
        .compile("jetpwmon");

    println!("cargo:rustc-link-search=native={}", std::env::var("OUT_DIR").unwrap());
    println!("cargo:rustc-link-lib=static=jetpwmon");
    println!("cargo:rustc-link-lib=pthread");


    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=vendor/src/jetpwmon.c");
    println!("cargo:rerun-if-changed=vendor/include/jetpwmon.h");
} 