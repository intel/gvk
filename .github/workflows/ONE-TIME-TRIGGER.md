# Making check-environment.yml Visible - One-Time Trigger

## What We're Doing

Adding a temporary push trigger to `check-environment.yml` so it runs once and becomes visible in the GitHub Actions UI.

## What I Changed

### Before:
```yaml
on:
  workflow_dispatch:  # Manual trigger only
```

### Now:
```yaml
on:
  workflow_dispatch:  # Manual trigger only
  push:
    branches:
      - setup-github-actions
    paths:
      - '.github/workflows/check-environment.yml'  # Only this file
```

## Why This Works

- The `paths:` filter means it ONLY triggers when `check-environment.yml` itself changes
- Won't trigger on other pushes
- After it runs once, the workflow becomes visible
- Then we can remove the push trigger

## What to Do

### Step 1: Commit and Push This Change
```sh
cd C:\Development\gvk-publish\gvk-public

git add .github/workflows/check-environment.yml
git commit -m "Temporarily trigger check-environment to make it visible"
git push origin setup-github-actions
```

**Result:** This push will trigger ONLY `check-environment.yml` (because we're changing that file)

### Step 2: Watch It Run
1. Go to GitHub ? Actions tab
2. You'll see "Check Environment" running
3. Wait for it to complete (~30 seconds)

### Step 3: Remove the Trigger (After It Runs)

After the workflow completes and becomes visible, remove the push trigger:

```yaml
on:
  workflow_dispatch:  # Manual trigger only
  # push:  # REMOVED - no longer needed
  #   branches:
  #     - setup-github-actions
```

Commit and push again:
```sh
git add .github/workflows/check-environment.yml
git commit -m "Remove temporary push trigger from check-environment"
git push origin setup-github-actions
```

**Result:** Back to manual-only, but now the workflow is visible!

## Safety Note

This is safe because:
- ? Only triggers when `check-environment.yml` itself changes (not other files)
- ? check-environment.yml doesn't install anything
- ? It's a read-only check
- ? We'll remove the trigger after it runs once

## What This Workflow Does

When it runs, it will:
- ? Show system information
- ? Check if Docker is available
- ? List installed build tools
- ? Show missing dependencies
- ? Install NOTHING

Completely safe!

## Timeline

```
1. Push this change
   ?
2. check-environment.yml runs (triggered by changing itself)
   ?
3. Workflow becomes visible in GitHub UI
   ?
4. Remove the push trigger
   ?
5. Back to manual-only, but now visible!
```

## Next Step

**Run these commands now:**

```sh
cd C:\Development\gvk-publish\gvk-public
git add .github/workflows/
git commit -m "Temporarily trigger check-environment to make it visible"
git push origin setup-github-actions
```

Then watch GitHub Actions! ??
