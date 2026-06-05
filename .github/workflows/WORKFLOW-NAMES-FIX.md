# Workflow Display Names - Fixed

## The Issue You Noticed

In GitHub Actions UI, you saw:
- ? "Check Environment" - User-friendly name
- ? ".github/workflows/build-and-test.yml" - File path

## Why This Happened

The `build-and-test.yml` file had a **duplicate `name:` field**:

```yaml
name: Build & Test
name: Build & Test  # ? Duplicate! Causes GitHub to ignore it
```

This syntax error caused GitHub to fall back to showing the file path.

## What I Fixed

Removed the duplicate:

```yaml
name: Build & Test  # ? Only one now!

on:
  workflow_dispatch:
  ...
```

## What You Should See After Pushing

After you commit and push this fix, GitHub Actions UI should show:

- ? **"Build & Test"** - User-friendly name
- ? **"Check Environment"** - User-friendly name
- ? **"System Report"** - User-friendly name (when it appears)

## Why "System Report" Might Not Appear Yet

The `system-report.yml` workflow won't show in the sidebar until:
1. You push it to GitHub, AND
2. It runs at least once

Same GitHub behavior we saw with `check-environment.yml` before.

## Next Steps

### 1. Commit the Fix
```sh
git add .github/workflows/build-and-test.yml
git commit -m "Fix duplicate name field in build-and-test workflow"
git push origin setup-github-actions
```

### 2. Refresh GitHub Actions
After pushing:
- Go to GitHub ? Actions
- Refresh the page
- You should now see "Build & Test" instead of the file path

### 3. Make System Report Visible

If you don't see "System Report" yet, use the direct URL:
```
https://github.com/intel/gvk/actions/workflows/system-report.yml
```
Then click "Run workflow"

## Current Workflows

After the fix, you should have:

| Workflow | Display Name | Status |
|----------|-------------|---------|
| `build-and-test.yml` | **Build & Test** | Fixed, will show name ? |
| `check-environment.yml` | **Check Environment** | Visible ? |
| `system-report.yml` | **System Report** | Needs first run |

---

**Push the fix now:**
```sh
git add .github/workflows/
git commit -m "Fix workflow display name"
git push origin setup-github-actions
```
