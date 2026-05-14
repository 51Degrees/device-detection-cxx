param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [string]$BuildMethod = "cmake"
)

# This is common logic, so let's call the common script
# Enable AddressSanitizer for test builds. The publish path goes through
# ci/build-package.ps1, which does not use this script, so production
# packages stay ASAN-free.
./cxx/build-project.ps1 -RepoName $RepoName -ProjectDir $ProjectDir -Name $Name -Configuration $Configuration -BuildMethod $BuildMethod -ExtraArgs '-DWITH_ASAN=ON'

exit $LASTEXITCODE
