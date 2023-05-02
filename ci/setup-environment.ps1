param(
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [string]$BuildMethod = "cmake"
)

if ($BuildMethod -eq "msbuild") {

    ./environments/setup-msbuild.ps1

}
