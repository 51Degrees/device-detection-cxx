param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [string]$BuildMethod = "cmake"
)

$deviceDetectionData = "$PSScriptRoot/../device-detection-data"
$enterpriseFile = "$deviceDetectionData/TAC-HashV41.hash"
$enterpriseFileBackup = "$deviceDetectionData/TAC-HashV41.hash.bak"

# Run tests with enterprise data file (if present)
Write-Output "Running tests with enterprise data file..."
./cxx/run-unit-tests.ps1 -RepoName $RepoName -ProjectDir $ProjectDir -Name $Name -Configuration $Configuration -Arch $Arch -BuildMethod $BuildMethod -ExcludeRegex ".*Example.*" `
    -CoverageExcludeDirs 'fiftyone-common-c(xx)?-cov\.dir$'

$enterpriseExitCode = $LASTEXITCODE

# Run tests with lite data file (temporarily hide enterprise file)
if (Test-Path $enterpriseFile) {
    Write-Output "Running tests with lite data file..."
    Move-Item $enterpriseFile $enterpriseFileBackup -Force

    try {
        ./cxx/run-unit-tests.ps1 -RepoName $RepoName -ProjectDir $ProjectDir -Name $Name -Configuration $Configuration -Arch $Arch -BuildMethod $BuildMethod -ExcludeRegex ".*Example.*" `
            -CoverageExcludeDirs 'fiftyone-common-c(xx)?-cov\.dir$'
        $liteExitCode = $LASTEXITCODE
    }
    finally {
        # Restore enterprise file
        Move-Item $enterpriseFileBackup $enterpriseFile -Force
    }

    # Fail if either test run failed
    if ($enterpriseExitCode -ne 0) {
        Write-Output "Tests with enterprise data file failed with exit code $enterpriseExitCode"
        exit $enterpriseExitCode
    }
    if ($liteExitCode -ne 0) {
        Write-Output "Tests with lite data file failed with exit code $liteExitCode"
        exit $liteExitCode
    }

    Write-Output "All tests passed with both enterprise and lite data files"
    exit 0
}
else {
    Write-Output "Enterprise data file not found, only ran tests with available data file"
    exit $enterpriseExitCode
}
