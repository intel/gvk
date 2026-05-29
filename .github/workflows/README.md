# GitHub Actions Build Setup

This document explains the GitHub Actions workflow configuration for building gvk on self-hosted runners.

## Overview

The workflow supports three build configurations:

1. **Windows Build** (`build-windows`) - Builds on self-hosted Windows runner
2. **Linux Build** (`build-linux`) - Builds in a GitHub Actions container on self-hosted Linux runner
3. **Linux Build with Docker** (`build-linux-docker`) - Builds using Docker on self-hosted Linux runner

## Self-Hosted Runner Requirements

### Linux Runner

Your self-hosted Linux runner needs:

- Docker installed and running (for Docker-based builds)
- GitHub Actions runner agent installed and configured
- Runner labels: `self-hosted`, `linux`

#### Installing Docker on Linux Runner

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y docker.io
sudo systemctl start docker
sudo systemctl enable docker

# Add the runner user to docker group (to run without sudo)
sudo usermod -aG docker $USER
# Log out and back in for group membership to take effect
```

#### Verifying Docker Setup

```bash
docker --version
docker run hello-world
```

### Windows Runner

Your self-hosted Windows runner needs:

- Visual Studio 2022 with C++ build tools
- CMake 3.5 or later
- Python 3.x
- GitHub Actions runner agent installed and configured
- Runner labels: `self-hosted`, `windows`

## Build Job Details

### build-linux (Container-based)

This job runs the build inside a GitHub Actions container. The container image must be pre-built and available to the runner.

**Pros:**
- Cleaner isolation
- GitHub Actions manages container lifecycle

**Cons:**
- Requires pre-built container image with tag `gvk-build:latest`
- Container must be built separately before first run

**To use this job:**
1. Build the Docker image on your runner:
   ```bash
   cd /path/to/gvk-public
   docker build -f .github/workflows/Dockerfile.build -t gvk-build:latest .
   ```
2. Make sure the image is available when the workflow runs

### build-linux-docker (Docker on Host)

This job builds the Docker image as part of the workflow, then runs the build inside the container.

**Pros:**
- Self-contained - builds its own Docker image
- No pre-setup required beyond Docker installation
- Image is always up-to-date

**Cons:**
- Slightly longer build time (image build on every run)
- Uses more disk space

**This is the recommended approach for getting started.**

## Triggering Builds

Currently, the workflow is configured for manual triggers only (`workflow_dispatch`). This allows you to test the setup without automatic builds.

To trigger manually:
1. Go to your repository on GitHub
2. Click "Actions" tab
3. Select "Build & Test" workflow
4. Click "Run workflow" button

### Enabling Automatic Builds

Once you've verified everything works, uncomment these lines in the workflow file:

```yaml
on:
  push:
    branches:
      - trunk
      - github-actions  # For testing
  pull_request:
    branches:
      - trunk
  workflow_dispatch:
  schedule:
    - cron: '30 5 * * *' # Daily 5:30 AM UTC
```

## Build Configuration

The workflow configures the project with these CMake options:

- `CMAKE_BUILD_TYPE=Release`
- `gvk-default_ENABLED=OFF`
- `gvk-string_ENABLED=ON`
- `gvk-build-tests=ON`
- `gvk-build-samples=OFF`

To change these options, edit the `Configure CMake` step in the workflow file.

## Artifacts

Each build job uploads:

- **Build Artifacts**: Compiled binaries in the `install/` directory
- **Test Results**: Test outputs from CTest in `build/Testing/`

Artifacts are retained for 7 days and can be downloaded from the workflow run page.

## Troubleshooting

### Docker Permission Denied

If you see "permission denied" errors when running Docker:

```bash
sudo usermod -aG docker $USER
```

Then log out and back in, or restart the runner service.

### Container Image Not Found

For the `build-linux` job, if you see "image not found" errors:

```bash
# Build the image manually on the runner
docker build -f .github/workflows/Dockerfile.build -t gvk-build:latest .
```

Or just disable that job and use `build-linux-docker` instead.

### Workflow Not Triggering

Make sure:
1. The workflow file is on the default branch (trunk)
2. Your runner is online (check Settings ? Actions ? Runners)
3. Your runner has the correct labels (`self-hosted`, `linux` or `windows`)

## Simplifying the Workflow

If you only want one Linux build approach:

### Option 1: Keep Docker-based build (recommended)

Comment out or delete the `build-linux` job and update the status check:

```yaml
status-check:
  needs: [build-windows, build-linux-docker]
```

### Option 2: Keep container-based build

Comment out or delete the `build-linux-docker` job and update the status check:

```yaml
status-check:
  needs: [build-windows, build-linux]
```

Make sure to build the container image first as described above.

## Docker Image Maintenance

The Docker image is based on Ubuntu 22.04 and includes all necessary build dependencies. To update:

1. Edit `.github/workflows/Dockerfile.build`
2. Rebuild the image:
   ```bash
   docker build -f .github/workflows/Dockerfile.build -t gvk-build:latest .
   ```

The `build-linux-docker` job automatically rebuilds the image on each run, so no manual maintenance is needed for that job.
