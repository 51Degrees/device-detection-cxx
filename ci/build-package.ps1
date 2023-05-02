param(
    [string]$RepoName
)

./cxx/build-package.ps1 -RepoName $RepoName

exit $LASTEXITCODE
