param(
    [string]$RepoName
)

# This is common logic, so let's call the common script
./cxx/build-package.ps1 -RepoName $RepoName

exit $LASTEXITCODE
