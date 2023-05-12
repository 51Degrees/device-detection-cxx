param (
    [string]$RepoName
)

$PackagePath = [IO.Path]::Combine($pwd, "package")
$BuildPath = [IO.Path]::Combine($pwd, $RepoName, "build")
$BinPath = [IO.Path]::Combine($BuildPath, "bin")

mkdir $BuildPath

# Copy the prebuilt binaries to the build directory
Copy-Item -Recurse -Path $PackagePath -Destination $BinPath
Copy-Item -Path $PackagePath -Destination $BinPath -Recurse
