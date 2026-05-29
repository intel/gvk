# Your GitHub Actions Workflows

## Active Workflows

### 1. Check Environment ? (NEW!)
**File:** `check-environment.yml`

**Purpose:** Check your runner setup without installing anything

**When to use:**
- ? Before your first build
- ? After changing runner configuration
- ? To see if Docker is available
- ? Anytime you want to verify your setup

**Runtime:** ~30 seconds

**What it does:**
- Shows system information
- Checks for Docker (doesn't install it)
- Lists installed build tools
- Shows missing dependencies
- Gives recommendations

**Installs:** Nothing! Pure checking only.

---

### 2. Build & Test ???
**File:** `build-and-test.yml`

**Purpose:** Build and test on Windows and Linux

**When to use:**
- ? To build your project
- ? To run tests
- ? For continuous integration

**Runtime:** 
- Environment check: ~30 seconds
- Windows build: 5-15 minutes
- Linux build: 2-10 minutes (first run may be longer)

**What it does:**
1. **Environment Check** - Quick verification (NEW!)
2. **Windows Build** - Build on Windows runner
3. **Linux Build** - Build on Linux runner (native, no Docker)
4. **Status Check** - Final summary

**Installs:** 
- Missing build tools (first run only)
- Vulkan dependencies (first run only)

---

## Workflow Comparison

| Workflow | Purpose | Install Anything? | Requires Docker? | Runtime |
|----------|---------|-------------------|------------------|---------|
| **Check Environment** | Verify setup | ? No | ? No | 30 sec |
| **Build & Test** | Build project | ? Yes (if needed) | ? No | 10-20 min |

## Recommended Order

### First Time Setup
```
1. Check Environment     ? Start here! See what you have
   ?
2. Read the output      ? Understand your setup
   ?
3. Build & Test         ? Do your first build
   ?
4. Check artifacts      ? Verify it worked
```

### Regular Use
```
Just run "Build & Test"
(Environment check happens automatically)
```

## Quick Reference

### To check your setup (no changes):
```
Actions ? Check Environment ? Run workflow
```

### To build your project:
```
Actions ? Build & Test ? Run workflow
```

### To enable automatic builds:
Uncomment the `push:` section in `build-and-test.yml`

## File Reference

### Workflows (Active)
- ? `check-environment.yml` - Environment checker
- ? `build-and-test.yml` - Main build workflow

### Documentation
- ?? `START-HERE.md` - Begin here!
- ?? `SIMPLE-SETUP.md` - Setup guide
- ?? `ENVIRONMENT-CHECK-GUIDE.md` - How to read environment check
- ?? `WHATS-NEW-ENV-CHECK.md` - What changed recently
- ?? `CHOOSE-YOUR-APPROACH.md` - Native vs Docker
- ?? `WORKFLOWS-OVERVIEW.md` - This file!

### Docker-Related (Optional)
- ?? `Dockerfile.build` - If you want Docker later
- ?? `build-and-test-simplified.yml.example` - Docker workflow example
- ?? `setup-runner.sh` - Docker setup script
- ?? `QUICKSTART.md` - Docker quick start
- ?? `README.md` - Comprehensive docs (includes Docker)

### Backups
- ?? `build-and-test-no-docker.yml` - Reference copy

## What's What?

**New to GitHub Actions?** ? Read `START-HERE.md`

**Want to check setup?** ? Run "Check Environment" workflow

**Ready to build?** ? Run "Build & Test" workflow

**Curious about Docker?** ? Read `CHOOSE-YOUR-APPROACH.md`

**Having issues?** ? Check `SIMPLE-SETUP.md` troubleshooting section

**Need details?** ? Read `README.md` (comprehensive)

## Pro Tips

?? Run "Check Environment" first - it's quick and informative  
?? First build takes longer (package installation)  
?? Subsequent builds are much faster  
?? You don't need Docker (but can use it if you want)  
?? Environment check happens automatically in builds  

---

**Ready?** ? Go to Actions ? Check Environment ? Run workflow! ??
