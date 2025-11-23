# CMake toolchain file for WASI (WebAssembly System Interface)
# This configures CMake to cross-compile C/C++ code to WebAssembly using WASI SDK

set(CMAKE_SYSTEM_NAME WASI)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

# Set WASI SDK path
set(WASI_SDK_PREFIX "$ENV{HOME}/wasi-sdk")

# Set compilers
set(CMAKE_C_COMPILER "${WASI_SDK_PREFIX}/bin/clang")
set(CMAKE_CXX_COMPILER "${WASI_SDK_PREFIX}/bin/clang++")
set(CMAKE_AR "${WASI_SDK_PREFIX}/bin/llvm-ar")
set(CMAKE_RANLIB "${WASI_SDK_PREFIX}/bin/llvm-ranlib")

# Set sysroot
set(CMAKE_SYSROOT "${WASI_SDK_PREFIX}/share/wasi-sysroot")

# Set compiler flags for WASI
# Enable emulated signal and getpid support, define NO_THREADING
set(CMAKE_C_FLAGS_INIT "-D__wasi__ -D_WASI_EMULATED_SIGNAL -D_WASI_EMULATED_GETPID -DFIFTYONE_DEGREES_NO_THREADING")
set(CMAKE_CXX_FLAGS_INIT "-D__wasi__ -D_WASI_EMULATED_SIGNAL -D_WASI_EMULATED_GETPID -DFIFTYONE_DEGREES_NO_THREADING")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-lwasi-emulated-signal -lwasi-emulated-getpid -Wl,--export-table")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-lwasi-emulated-signal -lwasi-emulated-getpid -Wl,--export-table")

# WASI doesn't support shared libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)

# Don't try to run test executables (can't run WASM on host)
set(CMAKE_CROSSCOMPILING_EMULATOR "")

# Skip compiler checks (they don't work well for cross-compilation)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Search for programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# WASI doesn't support threads - set variables to skip thread detection
set(CMAKE_THREAD_LIBS_INIT "")
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 0)
set(THREADS_PREFER_PTHREAD_FLAG OFF)
set(Threads_FOUND TRUE)
set(CMAKE_HAVE_THREADS_LIBRARY 1)

# WASM has built-in atomic operations, doesn't need libatomic
set(GCCLIBATOMIC_LIBRARY "")
set(GCCLIBATOMIC_FOUND TRUE)
