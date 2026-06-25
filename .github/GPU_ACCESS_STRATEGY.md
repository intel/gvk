# GPU Access Strategy for GitHub Actions Self-Hosted Runners

## Problem

When running tests in Docker containers, GPU access is complicated:
- Containers need GPU drivers from the host
- Vulkan ICD (Installable Client Driver) configuration must be available
- Device passthrough (`--device=/dev/dri`) alone is insufficient
- Different GPUs (NVIDIA vs Intel) require different driver setups

The initial approach of using Ubuntu 22.04 containers with `mesa-vulkan-drivers` only provided software rendering (llvmpipe), not actual GPU access.

## Solution

**Build Job**: Uses Docker container
- Doesn't need GPU access
- Benefits from reproducible container environment
- Faster to install dependencies in fresh container each time

**Test Jobs**: Run directly on host (no container)
- Direct access to GPU hardware
- Host already has correct GPU drivers installed (NVIDIA or Intel)
- Vulkan ICDs already configured on host
- Simpler and more reliable

## Current Configuration

### Build Job
```yaml
build:
  runs-on: [self-hosted, Linux, X64]
  container:
    image: ubuntu:22.04
    options: --device=/dev/dri --cap-add CAP_PERFMON
```

### Test Jobs
```yaml
test:
  runs-on: 
    - self-hosted
    - Linux
    - X64
    - ${{ matrix.runner.label }}

  # NO container - run directly on host for GPU access
```

## Benefits

1. **Reliability**: Direct GPU access without container complexity
2. **Simplicity**: No need to mount host drivers into container
3. **Compatibility**: Works with both NVIDIA and Intel GPUs
4. **Performance**: No container overhead for test execution
5. **Maintenance**: Host GPU drivers managed by lab admins, not workflow

## Tradeoffs

### Build in Container ?
- **Pro**: Reproducible build environment
- **Pro**: Easy dependency installation
- **Pro**: Isolation from host
- **Con**: None (build doesn't need GPU)

### Test on Host ?
- **Pro**: Direct GPU access
- **Pro**: Reliable Vulkan access
- **Pro**: Works with all GPU types
- **Con**: Depends on host having required libraries
- **Con**: Less isolation (but tests are read-only anyway)

## Host Requirements

Test runners must have:
- GPU drivers installed (NVIDIA/Intel)
- Vulkan loader and ICDs configured
- Basic system libraries (libwayland, libxkbcommon, etc.)

These are already installed by lab admins on self-hosted runners.

## Alternative Approaches Considered

### ? Option 1: GPU-enabled Docker containers
- Requires NVIDIA Container Toolkit or similar
- Complex setup for different GPU vendors
- Still has driver version matching issues
- Overkill for test execution

### ? Option 2: Mount host drivers into container
- Fragile (driver paths differ by distro/GPU)
- Version mismatches between host and container
- Requires `volumes:` which GitHub Actions containers don't fully support

### ? Option 3: Current approach (container for build, host for test)
- Best of both worlds
- Leverages host GPU setup
- Simple and maintainable

## GPU Detection Output

With this approach, `vulkaninfo --summary` should show actual GPU hardware:

**NVIDIA Runner (godzilla):**
```
GPU0:
  deviceName         = NVIDIA RTX A6000
  deviceType         = PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
  vendorID           = 0x10de
  driverVersion      = ...
```

**Intel Runner (mage-b580):**
```
GPU0:
  deviceName         = Intel(R) Arc(TM) B580 Graphics
  deviceType         = PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
  vendorID           = 0x8086
  driverVersion      = ...
```

## Future: If Container GPU Access Needed

If we absolutely need containers for test jobs in the future:

1. Use NVIDIA GPU-optimized containers for NVIDIA runners
2. Use Intel GPU-ready containers for Intel runners  
3. Use matrix to select different container images per GPU type
4. Install GPU drivers during container setup

But for now, the host-based approach is simpler and more reliable.
