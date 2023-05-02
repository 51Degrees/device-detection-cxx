param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release"
)


$RepoPath = [IO.Path]::Combine($pwd, $RepoName, $ProjectDir, "build")

Write-Output "Entering '$RepoPath'"
Push-Location $RepoPath

try {

    Write-Output "Testing $($Options.Name)"

    ctest -C $Configuration -T test --no-compress-output --output-junit "../test-results/performance/$Name.xml" --tests-regex ".*Performance.*" --exclude-regex ".*HighPerformance.*"
}
finally {

    Write-Output "Leaving '$RepoPath'"
    Pop-Location

}

$RepoPath = [IO.Path]::Combine($pwd, $RepoName)
$PerfResultsFile = [IO.Path]::Combine($RepoPath, "test-results", "performance-summary", "results_$Name.json")

Push-Location $RepoPath
try {
    if ($(Test-Path -Path "test-results") -eq  $False) {
        mkdir test-results
    }

    if ($(Test-Path -Path "test-results/performance-summary") -eq  $False) {
        mkdir test-results/performance-summary
    }

    $OutputFile = [IO.Path]::Combine($RepoPath, "summary.json")
    $DataFile = [IO.Path]::Combine($RepoPath, "device-detection-data", "TAC-HashV41.hash")
    if ($IsWindows) {
        $PerfPath = [IO.Path]::Combine($RepoPath, "build", "bin", $Configuration, "PerformanceHashC.exe")
    }
    else {
        $PerfPath = [IO.Path]::Combine($RepoPath, "build", "bin", "PerformanceHashC")
    }
    . $PerfPath --json-output $OutputFile --data-file $DataFile
    $Results = Get-Content $OutputFile | ConvertFrom-Json -AsHashtable
    Write-Output "{
        'HigherIsBetter': {
            'DetectionsPerSecond': $($Results.InMemory.DetectionsPerSecond)
        },
        'LowerIsBetter': {
        }
    }" > $PerfResultsFile
}
finally {
    Pop-Location
}

exit $LASTEXITCODE
