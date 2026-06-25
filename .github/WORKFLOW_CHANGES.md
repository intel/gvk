# GitHub Actions Workflow - gvk-test-package Implementation

## Summary of Changes

Updated `.github/workflows/build-and-test-docker.yml` to properly use `gvk-test-package` for testing, matching the Jenkins CI pattern, with comprehensive test result reporting via JUnit XML.

## What Changed

### Before (Incorrect Approach)
- Built `gvk-string` target only
- Uploaded entire `build/` directory as artifact (~hundreds of MB)
- Ran tests using `ctest` from `build/` directory
- Tests were run against build artifacts, not a proper test package
- Minimal test result reporting in job summary

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
   - Execute `./gvk-string.tests --gtest_output=xml:test-results.xml` directly
   - Matches Jenkins behavior: `dir('build/gvk-test-package/') { sh "./${tests[i]}" }`
   - **Full verbose output to stdout** via `tee` (for drilling into individual test steps)
   - **JUnit XML to file** (for GitHub Actions test result integration)

4. **Enhanced Test Reporting**:
   - Parse JUnit XML for accurate test counts (tests, failures, errors)
   - Display summary with ?/? status indicators
   - Show pass/fail counts prominently
   - **Full test output visible in step logs** (not in summary to keep it clean)
   - **XML uploaded as artifact** for historical tracking
   - **Test results published to GitHub UI** via EnricoMi/publish-unit-test-result-action

## Test Output Strategy

### Dual Output Format:
1. **Console (stdout)**: Full googletest verbose output
   - Available in the "Run Tests" step logs
   - Click into individual test jobs to see detailed output
   - Includes all test case names, assertions, timing, etc.

2. **JUnit XML**: Structured test results
   - Generated via `--gtest_output=xml:test-results.xml`
   - Parsed for job summary statistics
   - Published to GitHub UI for annotations
   - Uploaded as artifact for historical records

### Job Summary Shows:

```markdown
## Test Results - godzilla (NVIDIA RTX 6000)

? **All tests passed!** (6 tests)

?? Detailed test results available in step logs above
```

Or if tests fail:

```markdown
## Test Results - godzilla (NVIDIA RTX 6000)

? **Some tests failed:** 2 failed, 1 errors, 3 passed (of 6 total)

?? Detailed test results available in step logs above
```

### GitHub Test Results Integration

The `publish-unit-test-result-action` creates:
- ? Test result annotations on workflow run
- ?? Test summary table on main workflow page
- ?? Drill-down into specific test failures
- ?? Historical test trends (if enabled)

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
6. **Better Visibility**: Clear pass/fail status on main workflow page
7. **Proper Exit Codes**: Test failures are properly reported via exit codes
8. **Full Debug Output**: Complete test logs available in step details
9. **Historical Tracking**: XML artifacts retained for 30 days
10. **GitHub Integration**: Native test result display in Actions UI

## Testing Strategy

- Tests run directly as executables (not through ctest)
- Each GPU gets a self-contained test package
- Tests are independent of build environment
- Matches how tests would be deployed in production
- `continue-on-error: true` allows both test jobs to run even if one fails
- Separate test result checks for each GPU (godzilla and mage-b580)

## Next Steps

Once this pattern is validated with gvk-string:
1. Expand to other modules (gvk-handles, gvk-structures, etc.)
2. Eventually enable all tests: `-Dgvk-default_ENABLED=ON`
3. Full test suite on both GPU runners
4. Consider enabling trend analysis in publish-unit-test-result-action
5. May want to aggregate test results across GPUs for overall pass/fail

## Googletest XML Output

All gtest executables support `--gtest_output=xml:<filename>` to generate JUnit-compatible XML:

```bash
./gvk-string.tests --gtest_output=xml:test-results.xml
```

This works because all tests link against `gtest_main` which provides this functionality automatically.

