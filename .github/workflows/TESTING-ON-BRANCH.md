# Testing Workflows on Your Branch

## Current Setup

Your branch: `setup-github-actions`

Workflows are now configured to run on your branch!

## How It Works Now

### Automatic Triggers (Enabled)
```yaml
on:
  push:
    branches:
      - trunk
      - setup-github-actions  # ? Your branch!
```

**This means:**
- ? Every push to `setup-github-actions` branch triggers a build automatically
- ? Every push to `trunk` triggers a build
- ? You can still trigger manually anytime

### Manual Triggers (Always Available)
- You can trigger workflows manually on any branch
- Useful for re-running without pushing new code

## Testing Workflow

### Step 1: Push Your Branch
```bash
cd C:\Development\gvk-publish\gvk-public

# Commit your workflow changes
git add .github/workflows/
git commit -m "Add Linux build with environment check"

# Push to GitHub
git push origin setup-github-actions
```

**What happens:** Build automatically starts! ??

### Step 2: Watch the Build
1. Go to GitHub
2. Click **Actions** tab
3. You'll see the workflow running
4. Click on it to see live progress

### Step 3: Iterate
Make changes and push again:
```bash
# Make changes to workflow files
git add .github/workflows/
git commit -m "Fix build step"
git push origin setup-github-actions
```

Each push triggers a new build automatically!

## Testing Just the Environment Check

### Option 1: Manual Trigger
```
GitHub ? Actions ? "Check Environment" ? Run workflow
Select branch: setup-github-actions
Click "Run workflow"
```

**Best for:** Quick checks without pushing code

### Option 2: Add Auto-Trigger
You could make the environment check run on push too:

```yaml
# In check-environment.yml
on:
  workflow_dispatch:
  push:
    branches:
      - setup-github-actions
```

**Best for:** Running on every push to verify setup

## Current Workflow Behavior

| Workflow | Auto-runs on push? | Manual trigger? |
|----------|-------------------|-----------------|
| Build & Test | ? Yes (your branch) | ? Yes |
| Check Environment | ? No (manual only) | ? Yes |

## Testing Scenarios

### Scenario 1: Quick Environment Check
```
Don't push anything
Go to Actions ? Check Environment ? Run on setup-github-actions
Wait 30 seconds
Review results
```

### Scenario 2: Test the Build
```bash
git push origin setup-github-actions
# Automatically triggers build
# Watch in Actions tab
```

### Scenario 3: Iterate on Workflow
```bash
# Make changes
vim .github/workflows/build-and-test.yml

# Commit and push
git add .github/workflows/
git commit -m "Try different approach"
git push origin setup-github-actions

# New build starts automatically
```

### Scenario 4: Test Without Triggering Build
```bash
# Add [skip ci] to commit message
git commit -m "Update docs [skip ci]"
git push origin setup-github-actions

# Build will NOT run
```

## Viewing Results

### Where to Find Output

**Actions Tab ? Your Workflow Run**

You'll see:
- ? Check Environment job
- ? Build Windows job  
- ? Build Linux job
- ? Status Check job

Click any job to see detailed logs.

### Downloading Artifacts

After a successful build:
1. Go to the workflow run
2. Scroll to bottom
3. See "Artifacts" section
4. Download:
   - `gvk-windows-x64`
   - `gvk-linux-x64`
   - `test-results-windows`
   - `test-results-linux`

## Common Issues

### "Workflow not found"
- Make sure you pushed the `.github/workflows/` files
- Check they're in the right location
- Refresh the Actions tab

### "No runners available"
- Your self-hosted runners need to be online
- Check: Settings ? Actions ? Runners
- Make sure they show "Idle" (green)

### "Permission denied"
- Runner might need sudo access
- Check runner user has proper permissions

### Build fails on first run
- First run installs packages (takes longer)
- Check the "Install Build Dependencies" step for errors
- This is normal!

## Disabling Auto-Trigger

If you want to test manually only, comment out the push section:

```yaml
on:
  # push:
  #   branches:
  #     - trunk
  #     - setup-github-actions
  workflow_dispatch:  # Manual trigger only
```

Then each build requires manual trigger via GitHub UI.

## Branch Testing Checklist

- [ ] Push branch to GitHub
- [ ] Verify workflow files appear in repo
- [ ] Check runners are online (Settings ? Actions ? Runners)
- [ ] Run "Check Environment" manually first
- [ ] Review environment check output
- [ ] Push code to trigger automatic build
- [ ] Watch build progress in Actions tab
- [ ] Check for errors in job logs
- [ ] Download and verify artifacts
- [ ] Test multiple pushes (iteration)
- [ ] Verify everything works as expected
- [ ] Merge to trunk when ready

## When to Merge to Trunk

Merge when:
- ? Environment check succeeds
- ? Windows build succeeds
- ? Linux build succeeds
- ? Tests pass
- ? Artifacts are generated correctly
- ? You've tested at least 2-3 pushes
- ? Everything works reliably

```bash
# Ready to merge!
git checkout trunk
git merge setup-github-actions
git push origin trunk
```

After merge, workflows will run on trunk automatically!

## Pro Tips

?? **Test incrementally** - Run environment check first, then full build  
?? **Check logs carefully** - First run will install packages  
?? **Use manual triggers** - Test without pushing code  
?? **Keep branch up to date** - Merge trunk regularly while testing  
?? **Use [skip ci]** - Skip builds for doc-only changes  
?? **Watch runner capacity** - Windows + Linux builds run in parallel  

---

**Ready to test?** Just push your branch and watch the magic happen! ??

```bash
git push origin setup-github-actions
```

Then go to **Actions** tab to watch it run!
