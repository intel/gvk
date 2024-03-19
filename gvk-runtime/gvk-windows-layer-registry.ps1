
$LayerRegistryPath = "HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\Vulkan\ExplicitLayers"
$DefaultHivFilePath = ".\VulkanExplicitLayers.hiv"

function Add-Entry {
    param([string]$entry)
    echo $entry
    reg add $LayerRegistryPath /v $entry /t REG_DWORD /d 0 /f
}

function Add-Directory {
    param([string]$directory)
    Get-ChildItem -Path $directory -Filter *.json -File -Name | ForEach-Object {
        Add-Entry("$directory\$_")
    }
}

function Get-FilePath([string]$filePath, [string]$defaultFilePath) {
    if ($filePath -eq $null) {
        $filePath = $defaultFilePath
    }
    echo $filePath
}

################################################################################
if ($args[0] -eq "update") {
    Add-Directory("$Env:VULKAN_SDK\Bin")
    Add-Directory("$Env:GVK_LAYER_PATH")

################################################################################
} elseif ($args[0] -eq "save") {
    $filePath = Get-FilePath($args[1], $DefaultHivFilePath)
    echo "saving $filePath"
    reg save  $LayerRegistryPath $filePath /y

################################################################################
} elseif ($args[0] -eq "restore") {
    $filePath = Get-FilePath($args[1], $DefaultHivFilePath)
    echo "restoring $filePath"
    reg restore $LayerRegistryPath $filePath

################################################################################
} elseif ($args[0] -eq "clear") {
    reg delete $LayerRegistryPath /va /f

################################################################################
} elseif ($args[0] -eq "query") {
    reg query $LayerRegistryPath

################################################################################
} else {
    echo ""
    echo "gvk-windows-layer-registry.ps1"
    echo "  Utilities for working with the Vulkan Windows layer registry"
    echo "  $LayerRegistryPath"
    echo ""
    echo "update"
    echo "  Adds all json entries found in directories specified by environment variables"
    echo "  VULKAN_SDK (with \Bin appended) and GVK_LAYER_PATH"
    echo ""
    echo "save <optional:$DefaultHivFilePath>"
    echo "  Saves registry to a given .hiv file"
    echo "  NOTE : The given file path must end with the .hiv extension"
    echo ""
    echo "restore <optional:$DefaultHivFilePath>"
    echo "  Restores registry from a given .hiv file"
    echo ""
    echo "clear"
    echo "  Clears all entries"
    echo ""
    echo "query"
    echo "  Outputs all entries"
    echo ""
}
