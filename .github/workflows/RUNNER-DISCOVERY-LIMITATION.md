# How GitHub Actions Picks Runners

## The Reality

**Each job runs on exactly ONE runner.**

When you specify:
```yaml
runs-on: [self-hosted, linux]
```

GitHub's behavior:
1. Looks for all runners with labels: `self-hosted` AND `linux`
2. **Picks ONE** that's available
3. Runs the job on that one runner

**It will NOT run on all matching runners automatically.**

## Why You're Only Seeing One Runner

Your `runner-discovery.yml` has:
```yaml
discover-linux-runners:
  runs-on: [self-hosted, linux]
```

This means:
- GitHub finds all Linux runners
- **Picks one** (probably the first available)
- Runs the job on only that one
- Reports only that one runner

## Solutions to Discover All Runners

### Option 1: Give Runners Unique Labels (Recommended)

If your 2 Linux runners have different labels, we can target them explicitly:

**Example - If your runners are labeled:**
- Runner 1: `[self-hosted, linux, runner-1]`
- Runner 2: `[self-hosted, linux, runner-2]`

**Then use a matrix:**
```yaml
jobs:
  discover-linux-runners:
    strategy:
      matrix:
        runner: [runner-1, runner-2]
    runs-on: [self-hosted, linux, ${{ matrix.runner }}]
```

This creates 2 jobs:
- Job 1: Runs on runner-1
- Job 2: Runs on runner-2

### Option 2: Use Different Label Combinations

If runners have different label combinations:
```yaml
strategy:
  matrix:
    include:
      - runner-label: datacenter-1
      - runner-label: datacenter-2
runs-on: [self-hosted, linux, ${{ matrix.runner-label }}]
```

### Option 3: Manual Trigger with Input

Create a workflow that you run manually for each runner:
```yaml
on:
  workflow_dispatch:
    inputs:
      runner-name:
        description: 'Runner to check'
        required: true
        type: choice
        options:
          - runner-1
          - runner-2

jobs:
  check-runner:
    runs-on: [self-hosted, linux, ${{ inputs.runner-name }}]
```

Run it twice, selecting each runner.

## What We Need to Know

To properly discover all your runners, I need to know:

**Do your runners have unique labels?**

For example:
- Runner 1: `self-hosted, linux, runner-1, datacenter-east`
- Runner 2: `self-hosted, linux, runner-2, datacenter-west`

If yes, I can create a matrix strategy.

If no (both just have `self-hosted, linux`), then we need to either:
1. Add unique labels to each runner
2. Accept that we'll only see one at a time

## How to Check Your Runner Labels

1. Go to: `https://github.com/intel/gvk/settings/actions/runners`
2. Click on each runner
3. Look at the "Labels" section
4. Share what labels each has

## Current Workaround

Since GitHub Actions can't discover all runners automatically, the typical approach is:

1. **Know your runners beforehand** (from Settings ? Actions ? Runners)
2. **Configure the workflow** to explicitly target each one
3. **Use matrix strategy** to run parallel jobs on each

## Example: If You Have 2 Runners

**Scenario:** You have 2 Linux runners with labels:
- `[self-hosted, linux, runner-a]`
- `[self-hosted, linux, runner-b]`

**Solution:**
```yaml
jobs:
  discover-all-linux:
    strategy:
      matrix:
        runner: [runner-a, runner-b]
    runs-on: [self-hosted, linux, ${{ matrix.runner }}]
    steps:
      - name: Report Runner
        run: echo "Running on $RUNNER_NAME"
```

This creates 2 parallel jobs, one on each runner!

## Bottom Line

**GitHub Actions limitation:**
- ? Cannot auto-discover all runners
- ? Each job = one runner
- ? Can use matrix to run on multiple runners (if labeled)

**What we need:**
- Your runner labels (check GitHub Settings)
- Then I can create a matrix workflow

**Current behavior:**
- `runs-on: [self-hosted, linux]` ? Picks ONE random Linux runner
- To run on BOTH ? Need matrix with unique labels

---

**Next step:** Check your runner labels at:
```
https://github.com/intel/gvk/settings/actions/runners
```

Share what labels each runner has, and I'll create a matrix strategy to discover both! ??
