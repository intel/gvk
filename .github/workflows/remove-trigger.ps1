# Script to remove the temporary trigger from check-environment.yml

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Removing Temporary Trigger" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$workspaceRoot = "C:\Development\gvk-publish\gvk-public"
Set-Location $workspaceRoot

Write-Host "This will remove the push trigger from check-environment.yml" -ForegroundColor Yellow
Write-Host "and restore it to manual-only." -ForegroundColor Yellow
Write-Host ""

$continue = Read-Host "Have you confirmed the workflow is now visible? (y/n)"
if ($continue -ne "y") {
    Write-Host ""
    Write-Host "Please verify the workflow is visible first:" -ForegroundColor Yellow
    Write-Host "1. Go to: https://github.com/intel/gvk/actions" -ForegroundColor White
    Write-Host "2. Check left sidebar for 'Check Environment'" -ForegroundColor White
    Write-Host "3. If visible, run this script again" -ForegroundColor White
    Write-Host ""
    exit 0
}

Write-Host ""
Write-Host "Reading check-environment.yml..." -ForegroundColor Yellow

$file = ".github/workflows/check-environment.yml"
$content = Get-Content $file -Raw

# Remove the push trigger block
$newContent = $content -replace "(?s)(workflow_dispatch:.*?)  push:.*?- '\.github/workflows/check-environment\.yml'  # Only this file\s+", "`$1"

if ($content -eq $newContent) {
    Write-Host "??  No changes needed - trigger might already be removed" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Current on: section:" -ForegroundColor Cyan
    $onSection = ($content -split "on:")[1] -split "jobs:")[0]
    Write-Host $onSection
    exit 0
}

Write-Host "Writing updated file..." -ForegroundColor Yellow
$newContent | Set-Content $file -NoNewline

Write-Host "? File updated" -ForegroundColor Green
Write-Host ""

Write-Host "New on: section:" -ForegroundColor Cyan
$onSection = ($newContent -split "on:")[1] -split "jobs:")[0]
Write-Host $onSection
Write-Host ""

Write-Host "Committing changes..." -ForegroundColor Yellow
git add $file
git commit -m "Remove temporary push trigger from check-environment"

Write-Host ""
Write-Host "Pushing to GitHub..." -ForegroundColor Yellow  
git push origin setup-github-actions

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "? DONE!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

Write-Host "check-environment.yml is now back to manual-only!" -ForegroundColor Green
Write-Host "You can trigger it anytime from the Actions UI." -ForegroundColor Green
Write-Host ""
