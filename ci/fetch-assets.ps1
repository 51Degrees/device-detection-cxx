
param (
    [string]$RepoName,
    [Parameter(Mandatory=$true)]
    [string]$DeviceDetection,
    [string]$DeviceDetectionUrl
)

# Fetch the TAC data file for testing with
./steps/fetch-hash-assets.ps1 -RepoName $RepoName -LicenseKey $DeviceDetection -Url $DeviceDetectionUrl

# Move the data file to the correct location
$DataFileName = "TAC-HashV41.hash"
$DataFileSource = [IO.Path]::Combine($pwd, $RepoName, $DataFileName)
$DataFileDir = [IO.Path]::Combine($pwd, $RepoName, "device-detection-data")
$DataFileDestination = [IO.Path]::Combine($DataFileDir, $DataFileName)
Move-Item $DataFileSource $DataFileDestination

# Get the evidence files for testing. These are in the device-detection-data submodule,
# But are not pulled by default.
Push-Location $DataFileDir
try {
    Write-Output "Pulling evidence files"
    git lfs pull -I "*.csv" 
    git lfs pull -I "*.yml"
} catch {
    #ignore git lfs if it fails
} finally {
    Pop-Location
}

if ($IsLinux) {
    ls -l $DataFileDir
}

exit 0 #to ignore git lfs errors
