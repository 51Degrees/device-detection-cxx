# 51Degrees Device Detection API

![51Degrees](https://51degrees.com/DesktopModules/FiftyOne/Distributor/Logo.ashx?utm_source=github&utm_medium=repository&utm_content=home&utm_campaign=c-open-source "Data rewards the curious") **Device Detection**

[C Documentation](https://51degrees.com/device-detection-cxx/modules.html) and the [C++ Documentation](https://51degrees.com/device-detection-cxx/namespaces.html).

The 51Degrees device detection API is built on the 51Degrees [common API](https://github.com/51Degrees/common-cxx).

# Pre-requisites

## Data File

In order to perform device detection, you will need to use a 51Degrees data file. 
This repository includes a free, 'lite' file in the 'device-detection-data' 
sub-module that has a significantly reduced set of properties. To obtain a 
file with a more complete set of device properties see the 
[51Degrees website](https://51degrees.com/pricing). 
If you want to use the lite file, you will need to install [GitLFS](https://git-lfs.github.com/).

For Linux:
```
sudo apt-get install git-lfs
git lfs install
```

Then, navigate to the device-detection-data directory and execute:

```
git lfs pull
```

## Fetching sub-modules

This repository has sub-modules that must be fetched.
If cloning for the first time, use:

```
git clone --recurse-submodules https://github.com/51Degrees/device-detection-cxx.git
```

If you have already cloned the repository and want to fetch the sub modules, use:

```
git submodule update --init --recursive
```

If you have downloaded this repository as a zip file then these sub modules need 
to be downloaded separately as GitHub does not include then in the archive. 
In particular, note that the zip download will contain links to LFS content,
rather than the files themselves. As such, these need to be downloaded individually.

## Build tools

### Windows

You will need either Visual Studio 2019 or the [C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) installed.
If you have Visual Studio Code, you'll still need to install the build tools from the link above.
- Minimum Platform Toolset Version `v142`
- Minimum Windows SDK Version `10.0.18362.0`

### Linux 

You will need:
- [CMake 3.24](https://cmake.org/) or greater installed to build the project. 
- A C++ compiler which supports C++17. The compiler and other build tools will be selected by CMake automatically based on your environment.


# Installing

## Using CMake

To build the make files required to build, open a `bash` or `Visual Studio Developer Command Prompt` terminal and run

```
mkdir build
cd build
cmake .. 
```

Note: on an x64 Windows system, it is necessary to add `-A x64` as CMake will build a Win32 Solution by default.

Note: when building arm64 binaries on x64 Mac system, it is necessary to add `-DBUILD_TESTING=OFF` as CTest ([ref](https://cmake.org/cmake/help/latest/module/CTest.html)) will otherwise fail the build due to inability to run test executables designated for different CPU.

Note: for maximum possible performance file based operation can be disabled by adding `-DMemoryOnly=YES` to the `cmake ..` command.

```
mkdir build
cd build
cmake .. -DMemoryOnly=YES
```

Then build the whole solution with

```
cmake --build . --config Release
```

Libraries are output to the `lib/` directory, and executables like examples and tests are output to the `bin/` directory.

## Using Visual Studio

Calling `CMake` in an MSVC environment (as described in the [Using CMake](#Using-CMake) section) will produce a Visual Studio solution with projects for all libraries, examples, and tests. However, it is preferable to use the dedicated Visual Studio solution in the `VisualStudio/` directory.

## Build Options

For build options, see [Common API](https://github.com/51Degrees/common-cxx/blob/main/README.md#build-options).

# Tests

All unit, integration, and performance tests are built using the [Google test framework](https://github.com/google/googletest).

## CMake

CMake automatically pulls in the latest Google Test from GitHub.

Building the project builds multiple test executables to the `bin/` directory: `HashTests` and any common testing included in the common-cxx library.

These can be run by calling
```
ctest
```

If CMake has been used in an MSVC environment, then the tests will be set up and discoverable in the Visual Studio solution `51DegreesDeviceDetection` created by CMake.

## Visual Studio

Tests in the Visual Studio solution automatically install the GTest dependency via a NuGet package. However, in order for the tests to show up in the Visual Studio test explorer, the [Test Adapter for Google Test](https://marketplace.visualstudio.com/items?itemName=VisualCPPTeam.TestAdapterforGoogleTest) extension must be installed.

The VisualStudio solution includes `FiftyOne.DeviceDetection.Hash.Tests`, which can be run through the standard Visual Studio test runner. 

## Code Coverage

When the project files are generated for a non-MSVC compiler (gcc/clang) with `CMAKE_BUILD_TYPE=Debug` f.e. by running: 
```
cmake . -DCMAKE_BUILD_TYPE=Debug
```

the generated targets will contain code coverage instrumentation instructions to be added to the libraries with `-cov` suffix and `HashTests` target will be linked with them. During build time `*.gcno` and empty `*.gcda` files are generated.  

During the test run  `*.gcda` files will be filled with coverage data.  

Install gcovr tool by running:
```
pip3 install gcovr
```

For best results make sure you have at least version 7.2.

The following command will generate a full coverage report: 
```
mkdir -p coverage; gcovr --html-detail -o coverage/coverage.html; open coverage/coverage.html
```

To check the summary run:
```
gcovr -r . --print-summary
```

# Referencing the API

## CMake

When building using CMake, static libraries are built in stages. These can be included in an executable just as they are in the examples. If these are included through a CMake file, the dependencies will be resolved automatically. However, if linking another way, all dependencies will need to be included from the `lib/` directory. For example, to use the Hash C API, the following static libraries would need to be included:
- `fiftyone-common-c`
- `fiftyone-device-detection-c`
- `fiftyone-hash-c`

and for the Hash C++ API, the following are needed in addition:
- `fiftyone-common-cxx`
- `fiftyone-device-detection-cxx`
- `fiftyone-hash-cxx`

The CMake project also builds a single shared library containing everything. This is named `fiftyone-device-detection-complete` and can be referenced by any C or C++ project.

## Visual Studio

The Visual Studio solution contains static libraries which have all the dependencies set up correctly, so referencing these in a Visual Studio solution should be fairly self explanatory.

# Examples

There are several examples available, the source files can be found in the `examples/` folder. The examples are written in C or CPP.

### Visual Studio
All the examples are available to run in the `VisualStudio/DeviceDetection.sln` solution.

## Device Detection Examples

|Language|Example|Description|
|--|-----------|--------|
|C/C++|GettingStarted|This example shows how to get set up with a Device Detection Cloud aspect engine and begin using it to process User-Agents and Client Hints.<br>Also showcase `query.51D_deviceId` high-priority evidence feature (streamlines profile selection).|
|C/C++|MatchMetrics|This example shows how to view metrics associated with the results of processing with a Device Detection aspect engine using the Hash algorithm.|
|C++|MetaData|This example shows how to interrogate the meta data associated with the contents of a Device Detection Hash data file.|
|C|MemHash|This example measures the memory usage of the Hash algorithm.|
|C|OfflineProcessing|This example shows how process data for later viewing using a Device Detection Hash data file.|
|C|Performance|Command line performance evaluation program which takes a file of user agents and returns a performance score measured in detections per second per CPU core.|
|C|ProcHash|Command line process which takes a user agent via stdin and return device properties via stdout.|
|C/C++|ReloadFromFile|This example illustrates how to reload the data file from the data file on disk without restarting the application.|
|C/C++|ReloadFromMemory|This example illustrates how to reload the data file from a continuous memory space that the data file was read into without restarting the application.|
|C/C++|StronglyTyped|This example  takes some common User-Agents and returns the value of the IsMobile property as a boolean.|
|C|MatchForDeviceId|Retrieve device by deviceId used as evidence. DeviceId may have been obtained previously and stored to later lookup the device properties.|
|C|FindProfiles|Find all profiles that match a certain property value - in this example we count the number of mobile (IsMobile=true) profiles|
