# Super Simple Setup Guide (No Docker Required!)

## Quick Environment Check First! ??

Before your first build, run the environment check:

1. Go to GitHub ? Actions ? **"Check Environment"**
2. Click "Run workflow"
3. Wait ~30 seconds
4. Review the output to see what's installed

This tells you:
- ? What build tools you already have
- ?? What will be installed automatically
- ?? If Docker is available (optional)

**Then proceed with the setup below.**

---

## What You Need

Just a Linux machine with:
- Ubuntu/Debian (or similar)
- Ability to run `sudo` commands
- GitHub Actions runner installed

**That's it!** No Docker, no containers, no complexity.

## How It Works

The workflow automatically installs everything it needs using `apt-get`. When you trigger a build:

1. Checks out your code
2. Installs build tools (cmake, ninja, gcc, etc.)
3. Installs Vulkan and graphics libraries
4. Builds your project
5. Runs tests
6. Uploads artifacts

## To Run Your First Build

### Step 1: Make sure your runner is online
- Go to your repo on GitHub
- Settings ? Actions ? Runners
- Your Linux runner should show "Idle" (green dot)

### Step 2: Trigger a build
- Go to Actions tab
- Click "Build & Test" workflow
- Click "Run workflow" button
- Click the green "Run workflow" button

### Step 3: Watch it work!
- Click on the running workflow
- You'll see "Build Windows" and "Build Linux" jobs
- Click "Build Linux" to watch the build in real-time

## What Happens on First Run

The first time, your Linux runner will install packages (takes 2-5 minutes):
```
Installing build-essential, cmake, ninja-build...
Installing Vulkan dependencies...
```

**This only happens once!** The packages stay installed on your runner.

## After First Run

Subsequent builds are much faster because packages are already installed. Only your code gets rebuilt.

## No Setup Required!

Unlike Docker approaches:
- ? No Docker to install
- ? No images to build
- ? No containers to manage
- ? No extra scripts to run

Just install the GitHub Actions runner and you're ready!

## Troubleshooting

### "sudo: command not found"
Your runner user needs sudo permissions. Add to sudoers:
```bash
sudo usermod -aG sudo <runner-user>
```

### "apt-get: command not found"
You're not on Ubuntu/Debian. Change the install step to use your package manager:
- Fedora/RHEL: Change `apt-get` to `dnf` or `yum`
- Arch: Change to `pacman`

### Build fails with "ninja: not found"
The package installation failed. Check the job logs for errors in the "Install Build Dependencies" step.

## Want to Customize?

Edit `.github/workflows/build-and-test.yml`:

### Change what gets built
Find the "Configure CMake" step and modify:
```yaml
-Dgvk-build-samples=ON    # Enable samples
-Dgvk-build-tests=OFF      # Disable tests
```

### Add more dependencies
Find "Install Build Dependencies" step and add more packages:
```yaml
sudo apt-get install -y \
  your-package-here \
  another-package
```

## That's It!

Really. No Docker needed. The workflow handles everything else automatically.

---

**Pro tip:** Once everything works, uncomment the `push:` section in the workflow file to automatically build on every commit!
