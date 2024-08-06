param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [string]$BuildMethod = "cmake"
)

./cxx/run-unit-tests.ps1 -RepoName $RepoName -ProjectDir $ProjectDir -Name $Name -Configuration $Configuration -Arch $Arch -BuildMethod $BuildMethod -ExcludeRegex ".*Example.*" `
    -CoverageExcludeDirs 'fiftyone-common-c(xx)?-cov\.dir$'

exit $LASTEXITCODE
