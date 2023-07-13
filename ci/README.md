# API Specific CI Approach

For the general CI Approach, see [common-ci](https://github.com/51degrees/common-ci).

The following secrets are required:
* `ACCESS_TOKEN` - GitHub [access token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens#about-personal-access-tokens) for cloning repos, creating PRs, etc.
    * Example: `github_pat_l0ng_r4nd0m_s7r1ng`
  
The following secrets are required to run tests:
* `DEVICE_DETECTION_KEY` - [license key](https://51degrees.com/pricing) for downloading assets (TAC hashes file and TAC CSV data file)
    * Example: `V3RYL0NGR4ND0M57R1NG`
 
The following secrets are optional:
* `DEVICE_DETECTION_URL` - URL for downloading the enterprise TAC hash file
    * Default: `https://distributor.51degrees.com/api/v2/download?LicenseKeys=DEVICE_DETECTION_KEY&Type=HashV41&Download=True&Product=V4TAC`

### Differences
- There are no packages produced by this repository, so the only output from the `Nightly Publish Main` workflow is a new tag and release.
- The package update step does not update dependencies from a package manager in the same way as other repos.

### Build Options

For additional build options in this repo see [common-ci/cxx](https://github.com/51Degrees/common-ci/tree/main/cxx#readme)

## Prerequisites

In addition to the [common prerequisites](https://github.com/51Degrees/common-ci#prerequisites), the following environment variables are required:
- DEVICE_DETECTION_KEY - License key to download a data file for testing
- DEVICE_DETECTION_URL - Url to download the data file from (optional)
