# Dynamic Runner Detection

## What I Just Created

A new workflow: **`runner-discovery.yml`** that automatically detects ALL your runners!

## How It Works

### Step 1: Parallel Discovery
- Tries to run on `[self-hosted, linux]`
- Tries to run on `[self-hosted, windows]`
- Each runner that exists reports itself

### Step 2: Data Collection
Each available runner reports:
- Runner name
- OS type and version
- Architecture
- Hostname
- Has Docker?
- Has CMake?
- Has GCC/Clang?

### Step 3: Summary Generation
- Counts total runners found
- Lists Linux runners (however many exist)
- Lists Windows runners (however many exist)
- Generates recommendations

## What You'll Get

A `runner-summary.md` showing:

```markdown
# All Available Runners Report

## Summary
- Total Runners Detected: 2
- Linux Runners: 2
- Windows Runners: 0

## Linux Runners (2)

### Runner: linux-runner-gkredling-local1.json
```json
{
  "runner_name": "runner1",
  "runner_os": "Linux",
  "has_docker": true,
  ...
}
```

### Runner: linux-runner-gkredling-local2.json
```json
{
  "runner_name": "runner2",
  "runner_os": "Linux",
  "has_docker": false,
  ...
}
```

## Next Steps
- ? Linux runners available (2)
- ? No Windows runners detected
- ?? Configure workflows for Linux-only
```

## Advantages

### ? Automatic Detection
- Discovers however many runners you have
- Doesn't assume quantity
- Works if you add/remove runners

### ? Parallel Execution
- All runners report simultaneously
- Fast (~30 seconds regardless of runner count)

### ? Graceful Handling
- Windows jobs marked `continue-on-error: true`
- Won't fail if Windows runners don't exist
- Works with any combination

### ? JSON Data
- Structured data for each runner
- Easy to parse programmatically
- Can be used for automated configuration

## How to Run

```sh
# Commit the new workflow
git add .github/workflows/runner-discovery.yml
git commit -m "Add dynamic runner discovery workflow"
git push origin setup-github-actions
```

**This will trigger automatically** (because we're pushing runner-discovery.yml)

Or run manually:
```
GitHub ? Actions ? "Runner Discovery" ? Run workflow
```

## What It Tells You

### For Each Runner:
- Name and labels
- Operating system
- Architecture
- Docker availability
- Build tool availability

### Overall:
- Total runner count
- Linux vs Windows breakdown
- Recommendations for workflow configuration

## Example Scenarios

### Scenario 1: You Have 2 Linux Runners
```
Total: 2
Linux: 2
Windows: 0

Recommendation: Linux-only workflows
```

### Scenario 2: You Have 1 Linux, 1 Windows
```
Total: 2
Linux: 1
Windows: 1

Recommendation: Multi-platform builds enabled!
```

### Scenario 3: You Have Multiple of Each
```
Total: 4
Linux: 2
Windows: 2

Recommendation: Can run parallel builds!
```

## Integration with Other Workflows

Once we know what runners you have, we can:
- Update `system-report.yml` to only target existing runners
- Update `build-and-test.yml` to match your setup
- Create matrix builds if you have multiple runners

## Next Steps

1. **Push this workflow:**
   ```sh
   git add .github/workflows/
   git commit -m "Add runner discovery workflow"
   git push origin setup-github-actions
   ```

2. **Let it run** (~30 seconds)

3. **Download `runner-summary-report`** artifact

4. **Share it with me!**

Then I can see:
- Exactly how many runners you have
- What type each is
- What capabilities each has
- How to optimally configure your workflows

## Files Created

- ? `runner-discovery.yml` - The discovery workflow
- ? `RUNNER-DISCOVERY.md` - This documentation

---

**Ready to discover your runners?**

```sh
git add .github/workflows/
git commit -m "Add dynamic runner discovery"
git push origin setup-github-actions
```

?? Let's see what you've got!
