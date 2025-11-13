# 51Degrees Device Detection - Rust FFI Example

This example demonstrates how to use the 51Degrees Device Detection C library from Rust via FFI bindings. It works for both **native** and **WebAssembly (WASM)** targets using the same source code.

## Features

- ✅ Safe Rust wrappers around C FFI
- ✅ Works on native platforms (x86_64, ARM64, etc.)
- ✅ Works in WebAssembly (WASI)
- ✅ Single codebase for both targets
- ✅ Device detection from User-Agent strings
- ✅ Property retrieval (IsMobile, PlatformName, BrowserName, etc.)

## Prerequisites

### For Native Build

- Rust toolchain (1.70+)
- CMake (3.10+)
- C compiler (GCC, Clang, or MSVC)
- 51Degrees Lite data file

### For WASM Build (Additional)

- WASI SDK 21+ installed at `~/wasi-sdk`
- `wasmtime` runtime for testing

## Building

### Build C Libraries

First, build the 51Degrees C libraries for your target platform.

#### Native Build

```bash
cd /path/to/device-detection-cxx
mkdir -p build-rust
cd build-rust
cmake ..
make -j4 fiftyone-hash-c
```

#### WASM Build

```bash
cd /path/to/device-detection-cxx
mkdir -p build-wasi
cd build-wasi
cmake -DCMAKE_TOOLCHAIN_FILE=../wasi-toolchain.cmake ..
make -j4 fiftyone-hash-c
```

### Build Rust Example

#### Native Build

```bash
cd rust-poc
cargo build --release
```

The binary will be at: `target/release/detect`

#### WASM Build

```bash
cd rust-poc
cargo build --target wasm32-wasip1 --release
```

The WASM module will be at: `target/wasm32-wasip1/release/detect.wasm`

## Running

### Native

Make sure the Lite data file is available at `../device-detection-data/51Degrees-LiteV4.1.hash` (relative to rust-poc directory), then:

```bash
./target/release/detect
```

### WASM

Run with `wasmtime`, mounting the data directory:

```bash
wasmtime \
  --dir=/path/to/device-detection-data::/data/device-detection-data \
  target/wasm32-wasip1/release/detect.wasm
```

Example with full path:

```bash
wasmtime \
  --dir=/Users/eugene/projects/51degrees/51drepos/device-detection-cxx/device-detection-data::/data/device-detection-data \
  target/wasm32-wasip1/release/detect.wasm
```

## Example Output

Both native and WASM versions produce identical output:

```
51Degrees Device Detection Example
===================================

Initialized 51Degrees engine successfully

Mobile Device (Android):
  User-Agent: Mozilla/5.0 (Linux; Android 9; SAMSUNG SM-G960U)...
  IsMobile: True | Platform: Android | Browser: Samsung Browser

Desktop Device (Windows):
  User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)...
  IsMobile: False | Platform: Windows | Browser: Chrome

iPhone:
  User-Agent: Mozilla/5.0 (iPhone; CPU iPhone OS 15_0 like Mac OS X)...
  IsMobile: True | Platform: iOS | Browser: Mobile Safari

Detection completed successfully!
```

## Project Structure

```
rust-poc/
├── src/
│   ├── main.rs          # Example code (works for both native & WASM)
│   ├── lib.rs           # Safe Rust wrappers (HashEngine, HashResults)
│   └── bindings.rs      # Platform-agnostic FFI bindings
├── build.rs             # Build script (links C libraries, generates bindings)
├── Cargo.toml           # Rust package configuration
├── wrapper.h            # C header for bindgen
└── README.md            # This file
```

## Key Implementation Details

### Platform Differences

The example uses conditional compilation for only two things:

1. **Data file path**:
   - Native: `../device-detection-data/51Degrees-LiteV4.1.hash`
   - WASM: `/data/device-detection-data/51Degrees-LiteV4.1.hash` (mounted via wasmtime)

2. **Config selection**:
   - Native: Uses `fiftyoneDegreesHashInMemoryConfig` (with property-value index)
   - WASM: Uses `fiftyoneDegreesHashInMemoryConfigNoIndex` (avoids WASM memory issues)

Everything else uses the same code!

### WASM Workaround

Due to a reference counting issue in WASM, the example keeps all `HashResults` objects alive until the end of execution. This prevents premature dataset resource cleanup that would cause subsequent detections to fail.

### Critical Fixes Applied

The C library required two fixes for WASM compatibility:

1. **Exception Handling**: Modified `FIFTYONE_DEGREES_EXCEPTION_OKAY` macro in `src/common-cxx/exceptions.h` to accept both `NOT_SET` and `SUCCESS` status codes.

2. **Property-Value Index**: Added `fiftyoneDegreesHashInMemoryConfigNoIndex` configuration in `src/hash/hash.c` to disable the optimization that causes memory access violations in WASM.

## Troubleshooting

### Native Build Issues

- **Missing symbols**: Make sure C libraries are built in `../build-rust/lib/`
- **Data file not found**: Check path in `src/main.rs` or provide correct relative path

### WASM Build Issues

- **WASI SDK not found**: Set `WASI_SDK_PATH` environment variable or install at `~/wasi-sdk`
- **Linker errors**: Rebuild C libraries with WASI toolchain in `../build-wasi/`
- **Runtime errors**: Make sure to mount the data directory correctly with wasmtime's `--dir` flag

## License

This example is part of the 51Degrees Device Detection project.
See the main project LICENSE file for details.
