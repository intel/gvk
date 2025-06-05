
$UninstallRegistryPath = "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"

function Get-Keys {
    $keys = @()
    Get-ChildItem -Path $UninstallRegistryPath -Recurse | ForEach-Object {
        $key = $_.Name
        Get-ItemProperty -Path $_.PSPath | ForEach-Object {
            if ($_.PSObject.Properties["DisplayName"].Value.Contains("Vulkan SDK")) {
                $keys += $key
                Write-Host ""
                Write-Host $key
                $_.PSObject.Properties | ForEach-Object {
                    if (-not $_.Name.StartsWith("PS")) {
                        Write-Host "    $($_.Name) : $($_.Value)"
                    }
                }
            }
        }
    }
    Write-Host ""
    Write-Host "Vulkan SDK entries present in the uninstall registry : $($keys.Count)"
    return $keys
}

################################################################################
if ($args[0] -eq "query") {
    $keys = Get-Keys

################################################################################
} elseif ($args[0] -eq "clear") {
    foreach ($key in Get-Keys) {
        Write-Host "Deleting $key"
        reg delete $key /va /f
    }

################################################################################
} else {
    Write-Host ""
    Write-Host "gvk-vulkan-sdk-installation-cleanup.ps1"
    Write-Host "  Utility for removing Vulkan SDK entries from the Windows uninstall registry"
    Write-Host "  $UninstallRegistryPath"
    Write-Host ""
    Write-Host "query"
    Write-Host "  Outputs all Vulkan SDK entries present in the uninstall registry"
    Write-Host ""
    Write-Host "clear"
    Write-Host "  Clears all Vulkan SDK entries from the uninstall registry"
    Write-Host ""
}
