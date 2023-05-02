
param (
    [string]$RepoName,
    [Parameter(Mandatory=$true)]
    [string]$DeviceDetection,
    [string]$DeviceDetectionUrl
)


./steps/fetch-hash-assets.ps1 -RepoName $RepoName -LicenseKey $DeviceDetection -Url $DeviceDetectionUrl

$DataFileName = "TAC-HashV41.hash"
$DataFileSource = [IO.Path]::Combine($pwd, $RepoName, $DataFileName)
$DataFileDir = [IO.Path]::Combine($pwd, $RepoName, "device-detection-data")
$DataFileDestination = [IO.Path]::Combine($DataFileDir, $DataFileName)

mv $DataFileSource $DataFileDestination

Push-Location $DataFileDir
try {
    Write-Output "Pulling evidence files"
    git lfs pull -I "*.csv" 
    git lfs pull -I "*.yml"
}
finally {
    Pop-Location
}
