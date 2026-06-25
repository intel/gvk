# GitHub Actions Workflow - gvk-test-package Implementation

## Summary of Changes

Updated `.github/workflows/build-and-test-docker.yml` to properly use `gvk-test-package` for testing, matching the Jenkins CI pattern.

## What Changed

### Before (Incorrect Approach)
- Built `gvk-string` target only
- Uploaded entire `build/` directory as artifact (~hundreds of MB)
- Ran tests using `ctest` from `build/` directory
- Tests were run against build artifacts, not a proper test package

### After (Correct Approach - Matches Jenkins)
1. **Build Step**: Build `gvk-string.tests` target
   - This automatically builds gvk-string library
   - CMake POST_BUILD commands copy test executables and shared libraries to `build/gvk-test-package/`
   - The `gvk-vulkan-sdk.cmake` script is also copied to test package

2. **Artifact Upload**: Upload only `build/gvk-test-package/` (~much smaller)
   - Contains only test executables, shared libraries, and helper scripts
   - Self-contained test environment

3. **Test Execution**: Run tests directly from `gvk-test-package/`
   - Download artifact to `gvk-test-package/` directory
   - Execute `./gvk-string.tests` directly (not via ctest)
   - Matches Jenkins behavior: `dir('build/gvk-test-package/') { sh "./${tests[i]}" }`

## How gvk-test-package Works

From `cmake/gvk.build.cmake` (lines 254-269):
- When a test target is built, CMake automatically:
  1. Copies test executable to `build/gvk-test-package/`
  2. If the target is a shared library, copies the .so file too
  3. Copies any .json layer descriptors

From `cmake/external/Vulkan.cmake` (line 10):
- The `gvk-vulkan-sdk.cmake` script is copied to test package for Vulkan SDK setup

## Benefits

1. **Smaller Artifacts**: Only test package is uploaded (~10-50 MB vs hundreds of MB)
2. **Faster Downloads**: Test jobs download less data
3. **Matches Jenkins**: Same test execution pattern as production CI
4. **Self-Contained**: Test package has everything needed to run tests
5. **Portable**: Test package could be run on any machine with Vulkan drivers

## Testing Strategy

- Tests run directly as executables (not through ctest)
- Each GPU gets a self-contained test package
- Tests are independent of build environment
- Matches how tests would be deployed in production

## Next Steps

Once this pattern is validated with gvk-string:
1. Expand to other modules (gvk-handles, gvk-structures, etc.)
2. Eventually enable all tests: `-Dgvk-default_ENABLED=ON`
3. Full test suite on both GPU runners
