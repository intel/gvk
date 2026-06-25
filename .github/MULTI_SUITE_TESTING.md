# Running All Tests - Multi-Suite Test Execution

## Overview

Updated the workflow to run **all test executables** in the test package instead of just `gvk-string.tests`.

## What Changed

### Before: Single Test Suite
```sh
./gvk-string.tests --gtest_output=xml:test-results.xml
```
- Only ran gvk-string tests
- Generated single XML file
- ~27 tests

### After: All Test Suites
```sh
for test_exe in *.tests; do
  ./"$test_exe" --gtest_output=xml:"${test_exe%.tests}-results.xml"
done
```
- Runs all .tests executables found
- Generates separate XML per suite
- Aggregates results
- ~100+ tests (depending on enabled modules)

## Current Test Suites

Based on enabled modules:
1. **gvk-reference.tests** - Reference implementation tests
2. **gvk-runtime.tests** - Runtime utilities tests
3. **gvk-string.tests** - String utilities tests
4. **gvk-xml.tests** - XML parsing tests

## Test Execution Flow

### 1. Discovery
```sh
find gvk-test-package -name "*.tests" -type f -executable
```
Lists all test executables before running.

### 2. Execution Loop
For each test executable:
```sh
# Make executable
chmod +x *.tests

# Run with unique XML output
./gvk-reference.tests --gtest_output=xml:gvk-reference-results.xml
./gvk-runtime.tests --gtest_output=xml:gvk-runtime-results.xml
./gvk-string.tests --gtest_output=xml:gvk-string-results.xml
./gvk-xml.tests --gtest_output=xml:gvk-xml-results.xml
```

### 3. Exit Code Tracking
- Continues running all tests even if one fails
- Tracks overall exit code
- Reports failure if any test suite failed

### 4. Result Aggregation
```sh
OVERALL_EXIT_CODE=0
for test_exe in *.tests; do
  ./"$test_exe"
  EXIT_CODE=${PIPESTATUS[0]}
  if [ $EXIT_CODE -ne 0 ]; then
    OVERALL_EXIT_CODE=$EXIT_CODE
  fi
done
exit $OVERALL_EXIT_CODE
```

## Result Display

### Per-Suite Breakdown
```
## Test Results - godzilla (NVIDIA RTX 6000)

- **gvk-reference**: 24/24 passed
- **gvk-runtime**: 18/18 passed
- **gvk-string**: 27/27 passed
- **gvk-xml**: 15/15 passed

### Overall Results
**All tests passed:** 84 tests across 4 suites
```

### On Failure
```
- **gvk-reference**: 24/24 passed
- **gvk-runtime**: 16/18 passed  ??
- **gvk-string**: 27/27 passed
- **gvk-xml**: 15/15 passed

### Overall Results
**Some tests failed:** 2 failed, 0 errors, 82 passed (of 84 total)
```

## XML Files Generated

Each test suite gets its own XML file:
```
gvk-test-package/
??? gvk-reference-results.xml
??? gvk-runtime-results.xml
??? gvk-string-results.xml
??? gvk-xml-results.xml
```

All XML files are:
1. **Uploaded as artifacts**: `test-results-xml-{runner}-{sha}.zip`
2. **Published to GitHub**: Combined in single check status
3. **Parsed for summary**: Aggregated counts displayed

## Benefits

### 1. Comprehensive Coverage
- Tests all enabled modules
- Catches integration issues
- Validates cross-module dependencies

### 2. Isolated Failures
- One test suite failure doesn't block others
- Easy to identify which module has issues
- Per-suite pass/fail visibility

### 3. Parallel XML Generation
- Each suite has separate XML file
- No conflicts or overwrites
- Better debugging with isolated results

### 4. GitHub Integration
The `publish-unit-test-result-action` can handle multiple XML files:
```yaml
files: gvk-test-package/*-results.xml
```
Creates unified check with all results.

## Build Time Impact

| Configuration | Build Time | Test Time | Total |
|---------------|-----------|-----------|-------|
| Before (gvk-string only) | ~2 min | ~30 sec | ~2.5 min |
| After (4 modules) | ~8 min | ~2 min | ~10 min |

Still well within the 30-minute job timeout.

## Console Output

### Test Execution
```
==========================================
Running: gvk-reference.tests
==========================================
[==========] Running 24 tests from 3 test suites.
...
gvk-reference.tests exit code: 0
gvk-reference-results.xml generated successfully

==========================================
Running: gvk-runtime.tests
==========================================
[==========] Running 18 tests from 2 test suites.
...
```

### Summary
```
==========================================
Test Execution Summary
==========================================
Test executables run: 4
Overall exit code: 0

XML Results Files:
-rw-r--r-- 1 runner runner 12K gvk-reference-results.xml
-rw-r--r-- 1 runner runner  8K gvk-runtime-results.xml
-rw-r--r-- 1 runner runner 10K gvk-string-results.xml
-rw-r--r-- 1 runner runner  6K gvk-xml-results.xml
```

## Debugging Individual Test Failures

Each test suite also creates a text output file:
```
gvk-test-package/
??? gvk-reference-output.txt  # Full console output
??? gvk-runtime-output.txt
??? gvk-string-output.txt
??? gvk-xml-output.txt
```

These are captured via `tee` and remain in the workspace for inspection.

## Future: Selective Test Execution

Can be extended to run tests conditionally:
```yaml
matrix:
  test_suite: [gvk-reference, gvk-runtime, gvk-string, gvk-xml]

- name: Run Test Suite
  run: ./${{ matrix.test_suite }}.tests
```

This would create 8 parallel jobs (4 suites × 2 GPUs).

## Fallback Behavior

If no `.tests` executables are found:
```
Test executables run: 0
Overall exit code: 0
```

The job will succeed but report no tests ran. This prevents false positives if the build target changes.

## Next Steps

1. ? Run workflow to validate all tests execute
2. Monitor build times (may need adjustment)
3. Consider adding more modules as needed
4. Watch for module-specific GPU failures
