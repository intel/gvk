# Environment Check Workflow

## What It Does

Runs a quick (30 second) check of your Linux runner to see:

- ? What's already installed
- ?? What's missing
- ?? Whether Docker is available
- ?? Recommendations for your setup

**This workflow installs NOTHING** - it only checks what you have.

## How to Run It

1. Go to your repo on GitHub
2. Click **"Actions"** tab
3. Click **"Check Environment"** workflow (left sidebar)
4. Click **"Run workflow"** button (right side)
5. Click green **"Run workflow"** button
6. Wait ~30 seconds
7. Click on the workflow run to see results

## What the Output Tells You

### System Information
```
Hostname: your-runner-name
OS: Ubuntu 22.04.3 LTS
CPU: Intel(R) Core(TM) i7-9750H @ 2.60GHz
Memory: 16GB
```

Basic info about your runner machine.

### Docker Check

#### If you see ? Docker is installed and running:
```
? Docker is installed
? Docker daemon is running
? User is in docker group
```
**Meaning:** You COULD use Docker workflows if you wanted, but you don't need to!

#### If you see ? Docker is not installed:
```
? Docker is NOT installed
```
**Meaning:** Perfect! The native build workflow is ideal for you.

#### If you see ?? Docker installed but not running:
```
? Docker is installed
? Docker daemon is NOT running
```
**Meaning:** Docker is there but not started. Not a problem for native builds.

### Native Build Tools Check

#### If you see mostly ?:
```
? CMake 3.22.1
? Ninja: 1.10.0
? GCC 11.4.0
? Python: 3.10.12
```
**Meaning:** Great! First build will be fast since tools are already installed.

#### If you see some ?:
```
? CMake: Not installed
? Ninja: Not installed
? GCC 11.4.0
```
**Meaning:** No problem! The build workflow will install these automatically.

### Vulkan Dependencies Check

Shows which Vulkan development libraries are installed.

#### All installed:
```
? All Vulkan dependencies are installed!
```
**Meaning:** First build will be even faster!

#### Some missing:
```
??  Missing packages (5/8):
   libwayland-dev
   libxrandr-dev
   ...
```
**Meaning:** These will be installed automatically on first build.

### Recommendation Section

The workflow will suggest the best approach for your setup:

#### With Docker available:
```
?? You have Docker available!

You can choose between:
  1. Current workflow (native builds) - Simpler, works now
  2. Docker workflow - More isolated, reproducible

Recommendation: Stick with current workflow unless you
               specifically need Docker's features.
```

#### Without Docker:
```
? Native build approach is perfect for you!

Your current workflow will:
  1. Install missing packages automatically
  2. Build directly on the runner
  3. Work great without Docker
```

## Interpreting Results

### Perfect Setup (Nothing to do!)
```
? All core build tools are installed!
? All Vulkan dependencies are installed!
```
? Your first build will be fast (~2-5 minutes)

### Good Setup (Some tools missing)
```
??  Missing tools: cmake ninja-build
??  Missing packages (5/8): ...
```
? First build will take ~5 minutes (one-time package install)
? Subsequent builds will be fast

### Basic Setup (Most tools missing)
```
? CMake: Not installed
? Ninja: Not installed
...many missing packages...
```
? First build will take ~5-7 minutes (one-time package install)
? Subsequent builds will be fast

**All scenarios work fine!** The build workflow handles installation automatically.

## Common Questions

**Q: Should I install things manually before running the build?**
A: No need! The build workflow handles it. But if you want to speed up first build:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build
```

**Q: Do I need Docker?**
A: Nope! The current workflow works great without it.

**Q: What if Docker is available but I don't want to use it?**
A: Perfect! Just use the current workflow. No changes needed.

**Q: My environment check failed. Is that bad?**
A: Did it complete and show results? Then it worked! "Missing" tools are expected and fine.

**Q: How often should I run this check?**
A: Once is enough. Or run it again if you change your runner machine.

## Next Steps

After running the environment check:

1. ? Read the "Recommendation" section in the output
2. ? If it says "you're all set", proceed to run the build workflow
3. ? Go to Actions ? "Build & Test" ? Run workflow

That's it!
