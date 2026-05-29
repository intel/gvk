# Quick script to trigger workflows on your branch

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Triggering GitHub Actions Workflows" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$workspaceRoot = "C:\Development\gvk-publish\gvk-public"
Set-Location $workspaceRoot

# Check git status
Write-Host "Checking git status..." -ForegroundColor Yellow
$status = git status --porcelain
if ($status) {
    Write-Host "You have uncommitted changes:" -ForegroundColor Red
    git status --short
    Write-Host ""
    $continue = Read-Host "Commit these changes first? (y/n)"
    if ($continue -eq "y") {
        $message = Read-Host "Commit message"
        git add .
        git commit -m "$message"
    } else {
        Write-Host "Aborting. Please commit or stash your changes first." -ForegroundColor Red
        exit 1
    }
}

# Check current branch
$branch = git branch --show-current
Write-Host "Current branch: $branch" -ForegroundColor Green
Write-Host ""

if ($branch -ne "setup-github-actions") {
    Write-Host "Warning: You're not on setup-github-actions branch" -ForegroundColor Yellow
    $continue = Read-Host "Continue anyway? (y/n)"
    if ($continue -ne "y") {
        exit 0
    }
}

# Create a trigger file
Write-Host "Creating trigger commit..." -ForegroundColor Yellow
$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$triggerContent = @"
# Workflow Trigger

This file was created to trigger GitHub Actions workflows.

Triggered at: $timestamp
Branch: $branch

This file can be safely deleted after workflows appear in GitHub Actions UI.
"@

$triggerFile = ".github/workflows/TRIGGER.md"
$triggerContent | Out-File -FilePath $triggerFile -Encoding UTF8

# Commit and push
git add $triggerFile
git commit -m "Trigger workflow - test automation"

Write-Host ""
Write-Host "Pushing to GitHub..." -ForegroundColor Yellow
git push origin $branch

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "SUCCESS!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "What happens next:" -ForegroundColor Cyan
Write-Host "1. GitHub receives your push" -ForegroundColor White
Write-Host "2. build-and-test.yml starts automatically" -ForegroundColor White
Write-Host "3. Both workflows will appear in Actions UI" -ForegroundColor White
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Go to: https://github.com/intel/gvk/actions" -ForegroundColor White
Write-Host "2. You should see 'Build & Test' workflow running" -ForegroundColor White
Write-Host "3. After it starts, both workflows will be visible" -ForegroundColor White
Write-Host "4. You can then manually run 'Check Environment'" -ForegroundColor White
Write-Host ""
Write-Host "Press any key to open GitHub Actions in browser..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
Start-Process "https://github.com/intel/gvk/actions"
