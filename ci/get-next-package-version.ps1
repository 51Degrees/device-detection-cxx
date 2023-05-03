
param (
    [string]$RepoName,
    [Parameter(Mandatory=$true)]
    [string]$VariableName
)

# This is common logic, so let's call the common script
./steps/get-next-package-version.ps1 -RepoName $RepoName -VariableName "GitVersion"

# Set the variable for the caller
Set-Variable -Name $VariableName -Value $GitVersion.SemVer -Scope Global

exit $LASTEXITCODE
