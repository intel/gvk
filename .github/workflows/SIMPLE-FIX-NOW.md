# SIMPLE FIX: Run This Script

## The Problem
`check-environment.yml` exists but doesn't show in GitHub Actions UI.

## The Solution
Run one script to trigger it once, making it visible!

## What to Do

### Step 1: Run This Script
```powershell
cd C:\Development\gvk-publish\gvk-public
.\.github\workflows\trigger-once.ps1
```

**What it does:**
- Commits the temporary trigger
- Pushes to GitHub
- check-environment.yml runs automatically (ONCE)
- Opens GitHub Actions for you to watch

### Step 2: Wait for Workflow to Complete
- Go to: https://github.com/intel/gvk/actions
- Watch "Check Environment" run (~30 seconds)
- Wait for it to complete

### Step 3: Clean Up
```powershell
.\.github\workflows\remove-trigger.ps1
```

**What it does:**
- Removes the temporary trigger
- Back to manual-only
- Workflow stays visible!

## That's It!

After these 3 steps:
- ? check-environment.yml will be visible
- ? Manual trigger only
- ? Ready to use anytime

---

## Alternative: Manual Method

If you prefer to do it manually, see [ONE-TIME-TRIGGER.md](ONE-TIME-TRIGGER.md)

---

## What This Changes

The workflow will run ONCE to become visible. It's safe because:
- ? Only checks existing tools
- ? Installs nothing
- ? Read-only operation
- ? Takes 30 seconds

After it runs once, we remove the trigger and it's back to manual-only!
