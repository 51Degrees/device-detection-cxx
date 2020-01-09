# 51Degrees Device Detection API

![51Degrees](https://51degrees.com/DesktopModules/FiftyOne/Distributor/Logo.ashx?utm_source=github&utm_medium=repository&utm_content=home&utm_campaign=c-open-source "Data rewards the curious") **Device Detection**

[C Documentation](https://docs.51degrees.com/device-detection-cxx/4.0/modules.html) and the [C++ Documentation](https://docs.51degrees.com/device-detection-cxx/4.0/namespaces.html).

The 51Degrees device detection API provides the functionality of both Hash and Pattern device detection algorithms. All of the code shared by Hash and Pattern is in the [Common API](https://github.com/51Degrees/common-cxx/blob/master/readme.md) and the `src/` directory (depending on whether it is shared with other 51Degrees products or specific to device detection).

# Installing

## Using CMake

To build the make files required to build, open a `bash` or `Visual Studio Developer Command Prompt` terminal and run

```
mkdir build
cd build
cmake .. 
```
Note: on an x64 Windows system, it is neccessary to add `-A x64` as CMake will build a Win32 Solution by default.

Then build the whole solution with

```
cmake --build . --config Release
```

Libraries are output to the `lib/` directory, and executables like examples and tests are output to the `bin/` directory.

## Using Visual Studio

Calling `CMake` in an MSVC environment (as described in the [Using CMake](#Using-CMake) section) will produce a Visual Studio solution with projects for all libraries, examples, and tests. However, it is preferable to use the dedicated Visual Studio solution in the `VisualStudio/` directory.

## Build Options

For build options, see [Common API](https://github.com/51Degrees/common-cxx/blob/master/readme.md)

# Tests

All unit, integration, and performance tests are built using the [Google test framework](https://github.com/google/googletest).

## CMake

CMake automatically pulls in the latest Google Test from GitHub.

Building the project builds two test executables to the `bin/` directory: `HashTests` and `PatternTests`.

These can be run by calling
```
ctest
```

If CMake has been used in an MSVC environment, then the tests will be set up and discoverable in the Visual Studio solution `51DegreesDeviceDetection` created by CMake.

## Visual Studio

Tests in the Visual Studio solution automatically install the GTest dependency via a NuGet package. However, in order for the tests to show up in the Visual Studio test explorer, the [Test Adapter for Google Test](https://marketplace.visualstudio.com/items?itemName=VisualCPPTeam.TestAdapterforGoogleTest) extension must be installed.

The VisualStudio solution includes `FiftyOne.DeviceDetection.Hash.Tests` and `FiftyOne.DeviceDetection.Pattern.Tests`, which can both be run through the standard Visual Studio test runner. 

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

There are several examples available, the source files can be found in the `examples/` folder. The examples are written in C or CPP, for Hash or Pattern.

### Visual Studio
All the examples are available to run in the `VisualStudio/DeviceDetection.sln` solution.

## Device Detection Examples

|Example|Description|Language|Algorithm|
|-------|-----------|--------|---------|
|Getting Started|This example shows how to get set up with a Device Detection Cloud aspect engine and begin using it to process User-Agents.|C / C++|Hash / Pattern|
|Match For Device Id|This example shows how store device ids for processing later using a Device Detection aspect engine using the Pattern algorithm.|C|Pattern|
|Match Metrics|This example shows how to view metrics associated with the results of processing with a Device Detection aspect engine using the Hash algorithm.|C / C++|Hash / Pattern|
|Meta Data|This example shows how to interrogate the meta data associated with the contents of a Device Detection Hash data file.|C++|Hash / Pattern|
|MemHash|This example measures the memory usage of the Hash algorithm.|C|Hash|
|MemPat|This example measures the memory usage of the Pattern algorithm.|C|Pattern|
|Offline Processing|This example shows how process data for later viewing using a Device Detection Hash data file.|C|Hash / Pattern|
|PerfHash|Command line performance evaluation program which takes a file of user agents and returns a performance score measured in detections per second per CPU core.|C|Hash|
|PerfPat|Command line performance evaluation program which takes a file of user agents and returns a performance score measured in detections per second per CPU core.|C|Pattern|
|ProcHash|Command line process which takes a user agent via stdin and return device properties via stdout.|C|Hash|
|ProcPat|Command line process which takes a user agent via stdin and return device properties via stdout.|C|Pattern|
|Reload From File|This example illustrates how to reload the data file from the data file on disk without restarting the application.|C / C++|Hash / Pattern|
|Reload From Memory|This example illustrates how to reload the data file from a continuous memory space that the data file was read into without restarting the application.|C / C++|Hash / Pattern|
|Strongly Typed|This example  takes some common User-Agents and returns the value of the IsMobile property as a boolean.|C / C++|Hash / Pattern|
