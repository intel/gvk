# Script to trigger check-environment.yml once to make it visible

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Making check-environment.yml Visible" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$workspaceRoot = "C:\Development\gvk-publish\gvk-public"
Set-Location $workspaceRoot

Write-Host "Current location: $workspaceRoot" -ForegroundColor Yellow
Write-Host ""

# Check current state
Write-Host "Checking git status..." -ForegroundColor Yellow
$gitStatus = git status --porcelain .github/workflows/
if ($gitStatus) {
    Write-Host "Changes detected:" -ForegroundColor Green
    Write-Host $gitStatus
} else {
    Write-Host "No changes detected - that's unexpected!" -ForegroundColor Red
    Write-Host "The check-environment.yml should have been modified." -ForegroundColor Red
    Write-Host ""
    Write-Host "Please verify the file was changed correctly." -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "What will happen:" -ForegroundColor Cyan
Write-Host "1. Commit the temporary trigger change" -ForegroundColor White
Write-Host "2. Push to GitHub" -ForegroundColor White
Write-Host "3. check-environment.yml runs automatically (ONE TIME ONLY)" -ForegroundColor White
Write-Host "4. After it runs, the workflow becomes visible" -ForegroundColor White
Write-Host ""

$continue = Read-Host "Continue? (y/n)"
if ($continue -ne "y") {
    Write-Host "Cancelled." -ForegroundColor Yellow
    exit 0
}

Write-Host ""
Write-Host "Committing changes..." -ForegroundColor Yellow
git add .github/workflows/check-environment.yml .github/workflows/ONE-TIME-TRIGGER.md

try {
    git commit -m "Temporarily trigger check-environment to make it visible"
    Write-Host "? Committed successfully" -ForegroundColor Green
} catch {
    Write-Host "??  Commit may have failed, but continuing..." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Pushing to GitHub..." -ForegroundColor Yellow
git push origin setup-github-actions

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "? PUSHED!" -ForegroundColor Green  
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

Write-Host "What's happening now:" -ForegroundColor Cyan
Write-Host "1. GitHub received your push" -ForegroundColor White
Write-Host "2. check-environment.yml is being triggered" -ForegroundColor White
Write-Host "3. It will run in ~30 seconds" -ForegroundColor White
Write-Host "4. After it completes, it will be visible!" -ForegroundColor White
Write-Host ""

Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Go to: https://github.com/intel/gvk/actions" -ForegroundColor White
Write-Host "2. Watch 'Check Environment' workflow run" -ForegroundColor White
Write-Host "3. After it completes, refresh the page" -ForegroundColor White
Write-Host "4. It should now appear in the left sidebar!" -ForegroundColor White
Write-Host ""

Write-Host "After it appears, run this to remove the trigger:" -ForegroundColor Yellow
Write-Host "  .\.github\workflows\remove-trigger.ps1" -ForegroundColor White
Write-Host ""

$openBrowser = Read-Host "Open GitHub Actions in browser? (y/n)"
if ($openBrowser -eq "y") {
    Start-Process "https://github.com/intel/gvk/actions"
}

Write-Host ""
Write-Host "Done! Watch the Actions tab for the workflow run." -ForegroundColor Green
