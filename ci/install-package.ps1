param (
    [string]$RepoName
)

$PackagePath = [IO.Path]::Combine($pwd, "package")
$WorkDir = [IO.Path]::Combine($pwd, $RepoName)
$BuildDir = [IO.Path]::Combine($WorkDir, "build")


# Copy the prebuilt build dir with binaries and artifacts for tests into workdir:
Copy-Item -Path $PackagePath/* -Destination $WorkDir -Recurse

# We probably need just execution permissions (otherwise CTest fails to run HashTests binary), 777 to not bother:
chmod -R 777 $BuildDir

Write-Output "PackagePath contents:"
ls $PackagePath

Write-Output "WorkDir contents:"
ls $WorkDir
