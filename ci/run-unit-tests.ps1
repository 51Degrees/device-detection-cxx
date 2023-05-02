param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [string]$BuildMethod = "cmake"
)

$RepoPath = [IO.Path]::Combine($pwd, $RepoName, $ProjectDir, "build")

Write-Output "Entering '$RepoPath'"
Push-Location $RepoPath

try {

    Write-Output "Testing $($Options.Name)"

    # TODO ctest -C $Configuration -T test --no-compress-output --output-junit "../test-results/unit/$Name.xml" --exclude-regex ".*Example.*"
}
finally {

    Write-Output "Leaving '$RepoPath'"
    Pop-Location

}

exit $LASTEXITCODE
