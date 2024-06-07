
<###############################################################################

MIT License

Copyright (c) Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

###############################################################################>

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
