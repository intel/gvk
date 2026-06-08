# Docker-Based Build Workflow - Using GitHub Actions Container Feature

## What This Does

Based on PTI's approach, this uses GitHub Actions' **built-in container support** to:
1. Build once in Docker (clean, consistent environment)
2. Test on both GPUs (NVIDIA RTX 6000 and Intel B580)
3. No Docker permission issues!

## How It Works

### GitHub Actions Container Feature

```yaml
runs-on: [self-hosted, Linux]
container:
  image: ubuntu:22.04
  options: --device=/dev/dri

steps:
  - run: cmake ...  # Already inside container!
```

**Key difference from manual Docker:**
- ? Old way: `docker run ...` (requires permissions)
- ? New way: `container:` (GitHub Actions handles it)

## Workflow Structure

```
1. Build Job (any available runner)
   - Runs in Ubuntu 22.04 container
   - Installs dependencies
   - Builds with CMake + Ninja
   - Uploads build artifacts
   ?
2. Test Jobs (parallel)
   ?? test-godzilla (NVIDIA RTX 6000)
   ?  - Downloads build artifacts
   ?  - Runs tests on NVIDIA GPU
   ?  - Uploads test results
   ?
   ?? test-mage (Intel B580)
      - Downloads build artifacts
      - Runs tests on Intel GPU
      - Uploads test results
   ?
3. Status Check
   - Reports overall status
   - Warns if tests fail
```

## Key Features

### ? No Docker Permission Issues
GitHub Actions runner manages the containers - no need to fix permissions!

### ? Clean Environment
Every run starts with fresh Ubuntu 22.04 container - no state left on runners.

### ? GPU Passthrough
`--device=/dev/dri` gives container access to GPUs for Vulkan testing.

### ? Multi-GPU Testing
Builds once, tests on both NVIDIA and Intel GPUs automatically.

### ? Artifact Caching
Build artifacts shared between jobs - test jobs are fast.

## Configuration

### Build Configuration
```yaml
-DCMAKE_BUILD_TYPE=Release
-Dgvk-default_ENABLED=OFF
-Dgvk-string_ENABLED=ON
-Dgvk-build-tests=ON
-Dgvk-build-samples=OFF
```

### Container Options
```yaml
options: --device=/dev/dri --cap-add CAP_PERFMON
```
- `--device=/dev/dri` - GPU access
- `--cap-add CAP_PERFMON` - Performance monitoring (if needed)

## How to Run

```sh
git add .gitignore .github/workflows/build-and-test-docker.yml
git commit -m "Add Docker-based build workflow using container feature"
git push origin setup-github-actions
```

Or trigger manually:
```
GitHub ? Actions ? "Build and Test" ? Run workflow
```

## What Happens

1. **Build job** picks any available runner (godzilla or mage-b580)
2. GitHub Actions creates Ubuntu 22.04 container
3. Installs dependencies inside container
4. Builds project
5. Uploads artifacts

6. **Test jobs** run in parallel:
   - godzilla: Tests with NVIDIA RTX 6000
   - mage-b580: Tests with Intel B580

7. Each test job:
   - Downloads build artifacts
   - Runs in fresh Ubuntu 22.04 container with GPU access
   - Runs tests
   - Uploads results

## Expected Timeline

- **Build:** ~5-10 minutes (first time with dependency install)
- **Tests:** ~2-5 minutes each (parallel)
- **Total:** ~10-15 minutes

## Differences from PTI

### Similar:
- ? Uses `container:` feature
- ? Passes through GPU with `--device=/dev/dri`
- ? Self-hosted runners

### Different:
- PTI uses pre-built images with Intel OneAPI
- We use stock Ubuntu 22.04 and install dependencies
- PTI has matrix for multiple container versions
- We keep it simpler for now

## Future Enhancements

Once this works, we can:
1. Create custom Docker image (faster builds)
2. Add matrix for multiple Ubuntu versions
3. Add ARM runners when available
4. Enable auto-trigger on push/PR
5. Add performance tests
6. Add sanitizer builds (ASAN/TSAN)

## Troubleshooting

### If GPU access fails:
Check that runner can access `/dev/dri`:
```sh
ls -la /dev/dri
```

### If container pull fails:
GitHub Actions needs internet access to pull Ubuntu image. Check runner connectivity.

### If tests fail:
- Check test results artifacts
- GPU drivers might not be accessible in container
- Vulkan ICD might need host configuration

## Advantages Over Previous Approach

| Feature | Old (Manual Docker) | New (Container Feature) |
|---------|---------------------|-------------------------|
| Docker permissions | ? Required fix | ? No issue |
| Runner cleanup | ? Manual | ? Automatic |
| GPU access | ?? Complex | ? Simple option |
| Setup | ? Needs sudo | ? Works as-is |
| Maintenance | ? Runner config | ? Workflow only |

---

**Ready to test?**

```sh
git add .gitignore .github/workflows/build-and-test-docker.yml
git commit -m "Add Docker container-based build workflow"
git push origin setup-github-actions
```

Then go to Actions and run it! ??
