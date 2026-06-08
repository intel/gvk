# Workflow Configuration Summary

## Changes Made

### 1. All Report Workflows ? Manual Trigger Only

Changed these workflows to **manual trigger only** (no auto-run on push):
- ? `check-environment.yml`
- ? `multi-runner-system-report.yml`  
- ? `runner-discovery.yml`
- ? `system-report.yml`

**Why:** You already have the system reports, and can manually trigger them when needed.

### 2. Build & Test ? gvk-string Only

Updated `build-and-test-docker.yml` to:
- ? Build **only gvk-string** target
- ? Test **only gvk-string** tests (`ctest -R "gvk-string"`)
- ? Renamed to "Build and Test (gvk-string only)"

**Why:** Start small, validate the workflow works before expanding to full build.

## Current Workflow State

### Will Auto-Run on Push:
**NONE** - All workflows are manual trigger only! ?

### Available Manual Workflows:

| Workflow | Purpose | When to Use |
|----------|---------|-------------|
| **Build and Test (gvk-string only)** | Build & test gvk-string | Main CI workflow |
| Check Environment | Quick env check | Verify runner setup |
| Multi-Runner System Report | Detailed report from both runners | Compare runner configs |
| Runner Discovery | Detect available runners | See what runners exist |
| System Report | System report (older version) | Alternative to multi-runner |
| Build & Test (old) | Native build (no Docker) | Backup/reference |

## How to Run Build & Test

```
GitHub ? Actions ? "Build and Test (gvk-string only)" ? Run workflow
```

### What It Does:

```
1. Build Job
   ?? Run in Ubuntu 22.04 container
   ?? Install dependencies
   ?? Configure CMake (gvk-string enabled)
   ?? Build ONLY gvk-string target
   ?? Upload build artifacts

2. Test Jobs (Parallel)
   ?? godzilla (NVIDIA RTX 6000)
   ?  ?? Run ONLY gvk-string tests
   ?
   ?? mage-b580 (Intel B580)
      ?? Run ONLY gvk-string tests

3. Status Check
   ?? Report results
```

## Expected Results

### First Run:
- **Build:** ~5-10 minutes (installs dependencies)
- **Tests:** ~2-5 minutes each (parallel)
- **Total:** ~10-15 minutes

### Subsequent Runs:
- Dependency install is cached by APT within container
- Still ~10-15 minutes (container is fresh each time)

## gvk-string Configuration

```cmake
-DCMAKE_BUILD_TYPE=Release
-Dgvk-default_ENABLED=OFF      # Only what we explicitly enable
-Dgvk-string_ENABLED=ON         # Enable gvk-string
-Dgvk-build-tests=ON            # Build tests
-Dgvk-build-samples=OFF         # No samples
```

**Build target:** `gvk-string` (not `install`, not `all`)

**Test filter:** `ctest -R "gvk-string"` (only gvk-string tests)

## What Gets Tested

Only tests matching `gvk-string` in their name will run on both GPUs.

If tests fail:
- ? Workflow continues (continue-on-error)
- ?? Status shows warnings
- ?? Test results uploaded as artifacts

## Future Expansion

Once this works, you can expand to build more:

```cmake
# From:
-Dgvk-string_ENABLED=ON

# To:
-Dgvk-default_ENABLED=ON  # Enable everything
```

Or add specific targets:
```cmake
-Dgvk-string_ENABLED=ON
-Dgvk-structures_ENABLED=ON
-Dgvk-xml_ENABLED=ON
```

## Files Modified

1. `.gitignore` - Added pti-reference exclusion
2. `.github/workflows/build-and-test-docker.yml` - New Docker workflow (gvk-string only)
3. `.github/workflows/check-environment.yml` - Disabled auto-trigger
4. `.github/workflows/multi-runner-system-report.yml` - Disabled auto-trigger
5. `.github/workflows/runner-discovery.yml` - Disabled auto-trigger
6. `.github/workflows/system-report.yml` - Disabled auto-trigger

## Ready to Push

```sh
git status  # Review changes
git add .
git commit -m "Add gvk-string Docker build workflow, disable auto-triggers on reports"
git push origin setup-github-actions
```

**Then:** Manually trigger "Build and Test (gvk-string only)" from GitHub Actions UI! ??

---

**Nothing will auto-run on push** - you control when workflows execute! ?
