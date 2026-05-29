# Quick Fix: Make check-environment Appear

## The Problem
- ? check-environment.yml exists in your repo
- ? It doesn't show in GitHub Actions UI yet

## The Solution
Trigger it once via direct URL, then it will appear!

## Step-by-Step

### Method 1: Direct URL (Easiest)

1. **Copy this URL:**
   ```
   https://github.com/intel/gvk/actions/workflows/check-environment.yml
   ```

2. **Paste in your browser and press Enter**

3. **You'll see the workflow page with a "Run workflow" button**

4. **Click "Run workflow"**

5. **Select branch:** `setup-github-actions`

6. **Click the green "Run workflow" button**

7. **Done!** After it runs, it will appear in the left sidebar!

### Method 2: Use GitHub CLI (If Installed)

```sh
gh workflow run check-environment.yml --ref setup-github-actions
```

### Method 3: Temporarily Add Push Trigger

This will make it auto-trigger once, then you can remove it:

1. Edit `.github/workflows/check-environment.yml`

2. Change:
   ```yaml
   on:
     workflow_dispatch:
   ```

   To:
   ```yaml
   on:
     workflow_dispatch:
     push:
       branches:
         - setup-github-actions
   ```

3. Commit and push:
   ```sh
   git add .github/workflows/check-environment.yml
   git commit -m "Trigger check-environment once"
   git push origin setup-github-actions
   ```

4. Wait for it to run

5. Change it back:
   ```yaml
   on:
     workflow_dispatch:
   ```

6. Commit and push again

## Verification

After triggering, refresh the Actions page. You should see both workflows in the left sidebar:
- ? Build & Test
- ? Check Environment

## Why This is Needed

GitHub hides workflows until they've run at least once on a branch (unless they're on the default branch). This is normal GitHub behavior, not a bug!

Once it runs once, it stays visible. ?

---

**Recommended:** Use Method 1 (Direct URL) - fastest and easiest!
