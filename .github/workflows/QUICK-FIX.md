# START HERE - Quick Fix!

## Problem
You only see `build-and-test.yml` in GitHub Actions, not `check-environment.yml`.

## Why
GitHub hides workflows until they run at least once on your branch. **This is normal!**

## Solution (Pick One)

### FASTEST: Run the PowerShell Script
```powershell
cd C:\Development\gvk-publish\gvk-public
.\.github\workflows\trigger-workflows.ps1
```
Opens GitHub automatically when done!

### MANUAL: Push Any Change
```sh
cd C:\Development\gvk-publish\gvk-public
echo "# test" >> README.md
git add README.md
git commit -m "test workflows"
git push origin setup-github-actions
```

### Then
Go to **GitHub ? Actions tab** ? Both workflows appear! ?

## That's It!
Both workflow files exist and are correct. Just need to trigger them once.

---

**Read these for more info:**
- `ONLY-SEE-ONE-WORKFLOW.md` - Detailed explanation
- `TEST-SUMMARY.md` - How to test workflows
- `START-HERE.md` - Full overview
