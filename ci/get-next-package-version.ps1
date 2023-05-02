
param (
    [string]$RepoName,
    [Parameter(Mandatory=$true)]
    [string]$VariableName
)

./steps/get-next-package-version.ps1 -RepoName $RepoName -VariableName "GitVersion"

Set-Variable -Name $VariableName -Value $GitVersion.SemVer -Scope Global

exit $LASTEXITCODE
