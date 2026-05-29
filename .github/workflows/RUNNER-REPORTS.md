# Runner Report System

## Overview

I've created workflows that generate comprehensive reports about both your runners that you can download and share with me (or anyone else).

## Available Workflows

### 1. System Report (Comprehensive - Both Runners)
**File:** `system-report.yml`

**What it does:**
- ? Gathers detailed info from BOTH Windows and Linux runners
- ? Combines into a single downloadable report
- ? Shows hardware, software, build tools, everything!

**How to run:**
```
GitHub ? Actions ? "System Report" ? Run workflow
```

**What you get:**
- `combined-runner-report.md` - One file with everything from both runners

### 2. Check Environment (Linux Only - Quick)
**File:** `check-environment.yml`

**What it does:**
- ? Quick check of Linux runner only
- ? Focuses on build tools and Docker
- ? Generates a simple report

**How to run:**
```
GitHub ? Actions ? "Check Environment" ? Run workflow
```

**What you get:**
- `linux-environment-report.txt` - Text file with Linux runner details

## How to Get Reports

### Step 1: Trigger a Report Workflow

**For comprehensive report (both runners):**
```
1. Go to GitHub ? Actions
2. Click "System Report" (left sidebar)
3. Click "Run workflow"
4. Select branch: setup-github-actions
5. Click "Run workflow"
6. Wait 2-3 minutes
```

**For quick Linux check:**
```
1. Go to GitHub ? Actions
2. Click "Check Environment" (left sidebar)
3. Click "Run workflow"
4. Select branch: setup-github-actions
5. Click "Run workflow"
6. Wait 30 seconds
```

### Step 2: Download the Report

1. **Go to the completed workflow run**
   - Click on the workflow run in the Actions tab

2. **Scroll to bottom of the page**
   - You'll see "Artifacts" section

3. **Download the report:**
   - System Report: Click `combined-runner-report` (markdown file)
   - Check Environment: Click `linux-environment-report` (text file)

### Step 3: Share with Me

**Option 1: Paste in Chat**
- Open the downloaded file
- Copy the contents
- Paste here in the chat

**Option 2: Describe Key Points**
- Just tell me the important parts:
  - What build tools are installed/missing?
  - Is Docker available?
  - Any errors or issues?

## What's in Each Report

### System Report (combined-runner-report.md)

**Windows Runner Section:**
- System info (CPU, RAM, OS version)
- Graphics cards
- Disk space
- CMake version
- Visual Studio installations
- Python version
- Git version
- Environment variables
- Runner configuration

**Linux Runner Section:**
- System info (CPU, RAM, OS, kernel)
- Memory and disk
- Graphics hardware
- CMake, Ninja, GCC versions
- Python version
- **Docker status** (installed? running?)
- Vulkan/graphics libraries
- Environment variables
- Runner configuration

### Environment Check Report (linux-environment-report.txt)

**Quick snapshot:**
- System basics
- Docker availability
- Build tools status
- Vulkan dependencies
- Missing packages

## Using the Reports

### To Share with Me:

Just run "System Report", download `combined-runner-report.md`, and paste it here. I can then see:
- Exactly what you have installed
- What's missing
- Whether Docker is an option
- How to configure builds optimally

### To Understand Your Runners:

The reports show you:
- ? What's already installed
- ?? What needs to be installed
- ?? Docker status
- ?? What approach will work best

## Quick Commands

### Generate comprehensive report:
```
GitHub ? Actions ? System Report ? Run workflow
```

### Get Linux quick check:
```
GitHub ? Actions ? Check Environment ? Run workflow
```

### Download artifacts:
1. Click the workflow run
2. Scroll to bottom
3. Click artifact name under "Artifacts"

## Example: How I'd Use This

**You:**
1. Run "System Report"
2. Download `combined-runner-report.md`
3. Paste contents here

**Me:**
1. Review the report
2. See exactly what you have
3. Tell you:
   - What to enable/disable
   - Whether to use Docker
   - What packages are needed
   - Optimal build configuration

Much better than guessing! ??

## Files Created

- ? `system-report.yml` - Comprehensive report from both runners
- ? `check-environment.yml` - Updated with report artifact
- ? `RUNNER-REPORTS.md` - This guide

## Next Steps

1. **Commit these new files:**
   ```sh
   git add .github/workflows/
   git commit -m "Add runner report workflows"
   git push origin setup-github-actions
   ```

2. **Run System Report:**
   ```
   GitHub ? Actions ? System Report ? Run workflow
   ```

3. **Download and share the report with me!**

Then I can give you precise recommendations based on your actual setup! ??
