use std::env;
use std::path::PathBuf;

fn main() {
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    // Get the project root (parent directory of rust-poc)
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let project_root = PathBuf::from(&manifest_dir).parent().unwrap().to_path_buf();

    let common_cxx_dir = project_root.join("src/common-cxx");
    let hash_dir = project_root.join("src/hash");
    let src_dir = project_root.join("src");

    // Detect target architecture and environment
    let target_arch = env::var("CARGO_CFG_TARGET_ARCH").unwrap();
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();

    // Check if this is a WASM target (wasm32-unknown-unknown, wasm32-wasip1, or wasm32-unknown-emscripten)
    let is_wasm = target_arch == "wasm32";

    // Use different build directories for native vs WASM
    // For WASI targets, use build-wasi (proper WASI-compiled libraries)
    // For emscripten targets, use build-wasm (emscripten-compiled libraries)
    let (_build_dir, lib_dir) = if is_wasm {
        let build_dir = if target_os == "wasi" {
            project_root.join("build-wasi")
        } else {
            project_root.join("build-wasm")  // For emscripten
        };
        let lib_dir = build_dir.join("lib");
        println!("cargo:rerun-if-changed=../{}/lib/", build_dir.file_name().unwrap().to_str().unwrap());
        (build_dir, lib_dir)
    } else {
        let build_dir = project_root.join("build-rust");
        let lib_dir = build_dir.join("lib");
        println!("cargo:rerun-if-changed=../build-rust/lib/");
        (build_dir, lib_dir)
    };

    println!("cargo:rerun-if-changed=../src/");

    // Link against pre-built CMake libraries
    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=static=fiftyone-hash-c");
    println!("cargo:rustc-link-lib=static=fiftyone-device-detection-c");
    println!("cargo:rustc-link-lib=static=fiftyone-common-c");

    // Link against system libraries
    if !is_wasm && target_os != "windows" {
        println!("cargo:rustc-link-lib=dylib=m");
        println!("cargo:rustc-link-lib=dylib=pthread");
    } else if is_wasm && target_os == "wasi" {
        // Link against WASI emulated libraries for signal and getpid support
        let wasi_sdk_path = env::var("WASI_SDK_PATH")
            .unwrap_or_else(|_| format!("{}/wasi-sdk", env::var("HOME").unwrap()));
        let wasi_lib_path = format!("{}/share/wasi-sysroot/lib/wasm32-wasip1", wasi_sdk_path);
        println!("cargo:rustc-link-search=native={}", wasi_lib_path);
        println!("cargo:rustc-link-lib=static=wasi-emulated-signal");
        println!("cargo:rustc-link-lib=static=wasi-emulated-getpid");
        // Export function table to allow indirect function calls
        println!("cargo:rustc-link-arg=--export-table");
    }

    // Generate bindings
    // For WASM, use wrapper.h and just add NO_THREADING define
    // WASI provides standard libc, so no need for fake headers
    let wrapper_header = PathBuf::from(&manifest_dir).join("wrapper.h");

    let mut builder = bindgen::Builder::default()
        .header(wrapper_header.to_str().unwrap())
        .clang_arg(format!("-I{}", common_cxx_dir.display()))
        .clang_arg(format!("-I{}", hash_dir.display()))
        .clang_arg(format!("-I{}", src_dir.display()))
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()));

    // Always use allowlists to generate the needed bindings
    builder = builder
        .allowlist_function("fiftyoneDegrees.*")
        .allowlist_type("fiftyoneDegrees.*")
        .allowlist_var("FIFTYONE_DEGREES.*")
        .allowlist_var("fiftyoneDegrees.*Config")  // Allow config variables
        .allowlist_var("SUCCESS")
        .allowlist_var("e_fiftyone.*");  // Allow enum values

    // For WASM, add the NO_THREADING define and use WASI SDK headers for bindgen
    if is_wasm {
        // Point to WASI SDK sysroot for standard headers
        let wasi_sysroot = env::var("WASI_SYSROOT")
            .unwrap_or_else(|_| format!("{}/wasi-sdk/share/wasi-sysroot", env::var("HOME").unwrap()));

        builder = builder
            .clang_arg("-DFIFTYONE_DEGREES_NO_THREADING")
            .clang_arg("-D_WASI_EMULATED_SIGNAL")  // Enable signal emulation for headers
            .clang_arg(format!("--sysroot={}", wasi_sysroot))
            .clang_arg(format!("-I{}/include/wasm32-wasip1", wasi_sysroot));
    }

    // Use manually crafted bindings for both WASM and native
    // This keeps the code consistent across platforms
    let bindings_src = PathBuf::from(&manifest_dir).join("src/bindings.rs");
    std::fs::copy(&bindings_src, out_path.join("bindings.rs"))
        .expect("Failed to copy bindings");
}
