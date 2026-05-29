# You Only See build-and-test.yml? Here's Why!

## Quick Answer

`check-environment.yml` exists in your repo, but **GitHub doesn't show it yet** because:
- It hasn't run on your branch yet
- It's not on the trunk branch yet

**This is normal GitHub Actions behavior!**

## Quick Fix (30 Seconds)

### Option 1: Use PowerShell Script
```powershell
cd C:\Development\gvk-publish\gvk-public
.\.github\workflows\trigger-workflows.ps1
```

This will:
- Create a trigger commit
- Push to GitHub
- Open Actions page in browser

### Option 2: Manual Commands
```sh
cd C:\Development\gvk-publish\gvk-public

# Make any small change
echo "# Trigger" >> .github/workflows/TRIGGER.md

# Commit and push
git add .
git commit -m "Trigger workflows"
git push origin setup-github-actions
```

### Then:
1. Go to GitHub ? Actions tab
2. Watch `build-and-test.yml` start running
3. Refresh the page
4. Both workflows now appear!

## What You Currently Have

? Both workflow files exist on GitHub  
? Both are properly configured  
? Both are committed and pushed  

Just waiting for GitHub to discover them!

## Files on GitHub Right Now

You can verify they exist:
1. Go to: https://github.com/intel/gvk
2. Switch to branch: `setup-github-actions`
3. Browse to: `.github/workflows/`
4. You'll see:
   - `build-and-test.yml` ?
   - `check-environment.yml` ?
   - Other files ?

## Why This Happens

GitHub Actions workflow list is lazy-loaded:
- Workflows on trunk = always visible
- Workflows on other branches = visible after first run

**This is normal!** Not a bug in your setup.

## After Triggering

Once you push:
1. `build-and-test.yml` runs automatically
2. GitHub adds your branch to the workflow system
3. `check-environment.yml` appears in the UI
4. You can manually run it

## Verify Everything Is There

```sh
cd C:\Development\gvk-publish\gvk-public

# List workflow files
git ls-files .github/workflows/*.yml

# Expected output:
# .github/workflows/build-and-test.yml
# .github/workflows/check-environment.yml
# .github/workflows/build-and-test-no-docker.yml
```

## Don't Worry!

Your setup is correct. Just push something to trigger the workflows:

```sh
# Fastest way:
cd C:\Development\gvk-publish\gvk-public
powershell .\.github\workflows\trigger-workflows.ps1
```

Then check GitHub Actions tab! ??
