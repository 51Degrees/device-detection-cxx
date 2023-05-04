# API Specific CI Approach

For the general CI Approach, see [common-ci](https://github.com/51degrees/common-ci).

### Differences
- There are no packages produced by this repository, so the only output from the `Nightly Publish Main` workflow is a new tag and release.
- The package update step does not update dependencies from a package manager in the same way as other repos.

### Build Options

For additional build options in this repo see [common-ci/cxx](https://github.com/51Degrees/common-ci/tree/main/cxx#readme)

## Prerequisites

In addition to the [common prerequisites](https://github.com/51Degrees/common-ci#prerequisites), the following environment variables are required:
- DEVICE_DETECTION_KEY - License key to download a data file for testing
- DEVICE_DETECTION_URL - Url to download the data file from (optional)
