# Fix: Stale Test Results Being Published

## Problem

When the godzilla runner failed during artifact download (before tests ran), the test result steps still executed and showed:

```
? All tests passed: 27 tests
```

This was **misleading** because:
1. Tests never actually ran on godzilla (download failed)
2. The publish action found an old XML file from a previous run
3. The summary showed success even though the job failed

## Root Cause

The conditional `if: always()` on result steps meant they ran regardless of whether tests executed:

```yaml
- name: Run Tests (gvk-string)
  id: run-tests
  run: ./gvk-string.tests
  # This step FAILED (or was skipped due to download failure)

- name: Display Test Results
  if: always()  # ? Runs even when run-tests didn't execute!

- name: Publish Test Results
  if: always()  # ? Publishes old/stale XML file!
```

## Solution

Changed conditions to only run if the `run-tests` step actually executed:

```yaml
- name: Display Test Results
  if: always() && steps.run-tests.outcome != 'skipped'
  # ? Only runs if run-tests completed (success or failure)

- name: Upload Test Results (XML)
  if: always() && steps.run-tests.outcome != 'skipped'
  # ? Only uploads if tests actually ran

- name: Publish Test Results
  if: always() && steps.run-tests.outcome != 'skipped'
  # ? Only publishes fresh results, not stale ones
```

## How It Works

### Step Outcomes

GitHub Actions has these step outcomes:
- `success` - Step completed successfully
- `failure` - Step failed
- `skipped` - Step was skipped (prerequisite failed)
- `cancelled` - Step was cancelled

### Condition Logic

```yaml
if: always() && steps.run-tests.outcome != 'skipped'
```

This means:
- ? Run if tests **passed** (`success`)
- ? Run if tests **failed** (`failure`)  
- ? Don't run if tests were **skipped** (download failed, etc.)
- ? Don't run if tests were **cancelled**

## Scenarios

### Scenario 1: Normal Test Execution (Pass/Fail)

```
Download artifact  ? ? Success
Run tests         ? ? Success (or ? Failure)
Display results   ? ? Runs (shows actual results)
Upload XML        ? ? Runs (uploads fresh XML)
Publish results   ? ? Runs (publishes fresh results)
```

**Result:** Correct test results displayed

### Scenario 2: Download Failure (Godzilla Issue)

```
Download artifact  ? ? Failed
Verify package    ? ?? Skipped
Run tests         ? ?? Skipped (id: run-tests, outcome: skipped)
Display results   ? ?? Skipped (condition not met)
Upload XML        ? ?? Skipped (condition not met)
Publish results   ? ?? Skipped (condition not met)
```

**Result:** No misleading test results shown, job correctly fails

### Scenario 3: Test Executable Missing

```
Download artifact  ? ? Success
Verify package    ? ? Failed (executable not found)
Run tests         ? ?? Skipped
Display results   ? ?? Skipped (condition not met)
Upload XML        ? ?? Skipped (condition not met)
Publish results   ? ?? Skipped (condition not met)
```

**Result:** No misleading test results, job correctly fails

## Why `always()` is Still Needed

We still use `always()` in the condition because:

```yaml
if: always() && steps.run-tests.outcome != 'skipped'
```

- `always()` ? Run even if tests **failed**
- `&& steps.run-tests.outcome != 'skipped'` ? But only if tests actually ran

Without `always()`, the result steps would skip on test failures, and we'd never see failure reports.

## Expected Behavior Now

### When Godzilla Download Fails

```
Job: ? Test on godzilla (NVIDIA RTX 6000)
Summary: (No test results shown - correct!)
Status: Failed - Artifact download error
```

### When Tests Fail

```
Job: ? Test on godzilla (NVIDIA RTX 6000)
Summary: ? Some tests failed: 2 failed, 25 passed
Status: Failed - Tests failed
```

### When Tests Pass

```
Job: ? Test on godzilla (NVIDIA RTX 6000)  
Summary: ? All tests passed: 27 tests
Status: Success
```

## Related Issue

The stale XML file problem is also why we added:

```yaml
- name: Verify Test Package Downloaded
  run: |
    if [ ! -f "gvk-test-package/gvk-string.tests" ]; then
      echo "ERROR: Test executable not found!"
      exit 1
    fi
```

This ensures we fail fast if the test package isn't properly downloaded, preventing the test step from being skipped silently.

## Testing the Fix

To verify this fix works:

1. **Test with download failure**: Godzilla should show no test results
2. **Test with test failure**: Should show actual failure counts
3. **Test with test success**: Should show success normally

The key is that **stale/old results are never shown** - only fresh results from the current run.
