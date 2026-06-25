# GitHub Actions Matrix Strategy for Multi-Runner Testing

## Overview

The workflow now uses GitHub Actions **matrix strategy** to test on multiple GPU runners without duplicating job definitions.

## Configuration

### Matrix Definition

```yaml
test:
  name: Test on ${{ matrix.runner.name }}
  needs: build
  runs-on: [self-hosted, Linux, X64, ${{ matrix.runner.label }}]

  strategy:
    fail-fast: false  # Run all test jobs even if one fails
    matrix:
      runner:
        - name: "godzilla (NVIDIA RTX 6000)"
          label: nvidia-rtx6000
          check_name: "godzilla - NVIDIA RTX 6000"
        - name: "mage-b580 (Intel B580)"
          label: intel-b580
          check_name: "mage-b580 - Intel B580"
```

### Matrix Variables

- **`name`**: Human-readable runner name (used in job title and summaries)
- **`label`**: GitHub Actions runner label (used in `runs-on`)
- **`check_name`**: Name for test result checks (used in publish-unit-test-result-action)

## Benefits

1. **Single Job Definition**: Write test steps once, run on all runners
2. **Easy to Add Runners**: Just add a new entry to the `runner` array
3. **Parallel Execution**: All test jobs run simultaneously
4. **Independent Failures**: `fail-fast: false` ensures all tests run even if one fails
5. **Dynamic Runner Selection**: Can be extended with conditional logic

## Adding New Runners

To add a new test runner, simply add it to the matrix:

```yaml
matrix:
  runner:
    - name: "godzilla (NVIDIA RTX 6000)"
      label: nvidia-rtx6000
      check_name: "godzilla - NVIDIA RTX 6000"
    - name: "mage-b580 (Intel B580)"
      label: intel-b580
      check_name: "mage-b580 - Intel B580"
    # ADD NEW RUNNER HERE:
    - name: "your-runner (GPU Model)"
      label: your-runner-label
      check_name: "your-runner - GPU Model"
```

## Matrix Variable Usage

Throughout the job steps, reference matrix variables:

- Job name: `Test on ${{ matrix.runner.name }}`
- Runner selection: `runs-on: [self-hosted, Linux, X64, ${{ matrix.runner.label }}]`
- Artifact names: `test-results-xml-${{ matrix.runner.label }}-${{ github.sha }}`
- Test check name: `check_name: "Test Results (${{ matrix.runner.check_name }})"`
- Summary headers: `## Test Results - ${{ matrix.runner.name }}`

## Future: Dynamic Runner Selection

The matrix can be extended to support dynamic runner selection based on:

1. **Required Capabilities**: Filter runners by GPU features
2. **Workflow Inputs**: Let users select which runners to use
3. **Labels/Tags**: Use runner tags for capability matching
4. **Availability**: Check runner status before scheduling

### Example: Workflow Input for Runner Selection

```yaml
on:
  workflow_dispatch:
    inputs:
      runners:
        description: 'Runners to test on (comma-separated labels)'
        required: false
        default: 'nvidia-rtx6000,intel-b580'
```

Then use a setup job to build the matrix dynamically based on inputs.

## Status Job

The status job references the matrix test job:

```yaml
status:
  needs: [build, test]  # 'test' refers to all matrix instances

  steps:
    - name: Check Status
      run: |
        echo "Test: ${{ needs.test.result }}"  # Overall result of all matrix jobs
```

The `needs.test.result` will be:
- `success`: All matrix jobs passed
- `failure`: Any matrix job failed
- `cancelled`: Any matrix job was cancelled

## Removed Emoji Characters

The workflow previously used emoji characters (?, ?, ??, ??) which were displaying as `?` in the GitHub UI. These have been removed for better compatibility.

**Before:**
```
? **All tests passed!** (27 tests)
```

**After:**
```
**All tests passed:** 27 tests
```

## Current Runners

| Name | Label | GPU |
|------|-------|-----|
| godzilla | `nvidia-rtx6000` | NVIDIA RTX PRO 6000 |
| mage-b580 | `intel-b580` | Intel Arc B580 |

## Next Steps

1. **Validate** matrix strategy with current runners
2. **Add more runners** as they become available in the lab
3. **Implement** capability-based runner selection
4. **Consider** grouping runners by capabilities (e.g., all NVIDIA, all Intel)
5. **Add** conditional test execution based on runner type
