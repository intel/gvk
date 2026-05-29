# Quick Start Guide for Self-Hosted Linux Builds

## What Was Set Up

Your GitHub Actions workflow now has **two Linux build options**:

1. **build-linux** - Uses GitHub Actions container feature (requires pre-built image)
2. **build-linux-docker** - Builds Docker image and runs build inside it (recommended for beginners)

## Fastest Way to Get Started

### Step 1: Install Docker on Your Linux Runner

```bash
sudo apt-get update
sudo apt-get install -y docker.io
sudo systemctl start docker
sudo systemctl enable docker
sudo usermod -aG docker $USER
```

**Important:** Log out and back in (or restart the runner service) after adding user to docker group.

### Step 2: Test Docker

```bash
docker --version
docker run hello-world
```

### Step 3: Run the Workflow

Go to GitHub ? Actions ? Build & Test ? Run workflow

That's it! The **build-linux-docker** job will:
- Automatically build the Docker image
- Run the build inside the container
- Upload artifacts

## Optional: Pre-build the Docker Image

If you want to use the **build-linux** job (faster but requires setup):

```bash
cd /path/to/gvk-public
bash .github/workflows/setup-runner.sh
```

Or manually:

```bash
docker build -f .github/workflows/Dockerfile.build -t gvk-build:latest .
```

## Choose Your Approach

### Option A: Use Both (Current Setup)
- Pros: Test both approaches, redundancy
- Cons: Longer CI time, more resource usage

### Option B: Use Only Docker-Based Build (Recommended)
Replace your workflow with the simplified version:

```bash
cd .github/workflows
mv build-and-test.yml build-and-test-full.yml.backup
mv build-and-test-simplified.yml.example build-and-test.yml
```

This gives you just one Linux job that builds everything in Docker.

## Files Created

- **Dockerfile.build** - Docker image definition with all build dependencies
- **README.md** - Detailed documentation
- **setup-runner.sh** - Helper script to build Docker image
- **build-and-test-simplified.yml.example** - Simplified workflow option

## Troubleshooting

### "Docker: permission denied"
```bash
sudo usermod -aG docker $USER
# Then log out and back in
```

### "Cannot connect to Docker daemon"
```bash
sudo systemctl start docker
```

### Want to test locally first?
```bash
docker build -f .github/workflows/Dockerfile.build -t gvk-build:latest .
docker run -it --rm -v $(pwd):/workspace -w /workspace gvk-build:latest bash
# Inside container:
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release -Dgvk-default_ENABLED=OFF -Dgvk-string_ENABLED=ON
cmake --build build --parallel
```

## Next Steps

1. ? Install Docker on Linux runner
2. ? Test Docker with `docker run hello-world`
3. ? Trigger workflow manually to test
4. ?? Once working, uncomment automatic triggers in workflow
5. ?? Consider using simplified workflow if you don't need both Linux options

## Need Help?

Check `.github/workflows/README.md` for detailed documentation.
