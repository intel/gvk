# Quick Command Reference

## Your Current Branch
```bash
setup-github-actions
```

## Push and Test

### First Time Setup
```bash
cd C:\Development\gvk-publish\gvk-public

# Stage workflow files
git add .github/workflows/

# Commit
git commit -m "Add Linux build workflow with environment check"

# Push (triggers build automatically!)
git push origin setup-github-actions
```

### Watch the Build
```
Open browser ? Your GitHub repo ? Actions tab
```

## Iterate and Test

### Make changes and test again
```bash
# Edit workflow
notepad .github\workflows\build-and-test.yml

# Commit and push
git add .github\workflows\
git commit -m "Update workflow"
git push origin setup-github-actions
# Build starts automatically!
```

### Update documentation without triggering build
```bash
git add .github\workflows\*.md
git commit -m "Update docs [skip ci]"
git push origin setup-github-actions
# No build triggered
```

## Manual Workflow Triggers

### Trigger without pushing code
```
GitHub ? Actions ? Select workflow ? Run workflow
Choose branch: setup-github-actions
Click "Run workflow"
```

## Check Environment (Quick Test)

### Manual trigger (recommended first)
```
GitHub ? Actions ? "Check Environment" ? Run workflow
Branch: setup-github-actions
Wait: 30 seconds
```

## When Ready to Merge

### Merge to trunk
```bash
# Make sure branch is up to date
git checkout setup-github-actions
git pull origin setup-github-actions

# Switch to trunk
git checkout trunk
git pull origin trunk

# Merge your branch
git merge setup-github-actions

# Push to trunk (triggers build on trunk)
git push origin trunk
```

## Troubleshooting

### Check if runners are online
```
GitHub ? Settings ? Actions ? Runners
Should show "Idle" (green dot)
```

### View workflow files in GitHub
```
GitHub ? Code ? .github/workflows/ folder
Verify files are present
```

### Check recent builds
```
GitHub ? Actions tab
See all workflow runs
```

## Common Git Commands

### See current branch
```bash
git branch
```

### See current status
```bash
git status
```

### See recent commits
```bash
git log --oneline -5
```

### Undo last commit (keep changes)
```bash
git reset --soft HEAD~1
```

### Force push (use carefully!)
```bash
git push origin setup-github-actions --force
```

## Workflow Status

### Check if workflows are enabled
```
GitHub ? Actions tab
Should see workflow list
```

### Disable/Enable workflows
```
GitHub ? Actions ? Select workflow ? ? menu ? Disable/Enable
```

## Quick Tests

### 1. Environment Check Only
```
Actions ? Check Environment ? Run on setup-github-actions
```

### 2. Full Build
```
git push origin setup-github-actions
(or)
Actions ? Build & Test ? Run on setup-github-actions
```

### 3. Test Both
```
1. Run Check Environment (manual)
2. Wait for results
3. Push code (triggers Build & Test)
4. Watch both succeed
```

## File Locations

```
C:\Development\gvk-publish\gvk-public\
  ?? .github\
      ?? workflows\
          ?? build-and-test.yml          ? Main build (auto-runs)
          ?? check-environment.yml       ? Env check (manual)
          ?? START-HERE.md               ? Start guide
          ?? TESTING-ON-BRANCH.md        ? This guide expanded
          ?? *.md                        ? Documentation
```

## Next Steps

```bash
# 1. Push your branch
git push origin setup-github-actions

# 2. Watch it run
# GitHub ? Actions tab

# 3. Iterate as needed
# Edit ? Commit ? Push ? Repeat

# 4. Merge when ready
git checkout trunk
git merge setup-github-actions
git push origin trunk
```

---

**Everything is set up!** Just push and watch it work! ??
