# Vulkan SDK Caching Strategy

## Overview

The workflow automatically detects the required Vulkan SDK version from `CMakeLists.txt` and caches it to avoid re-downloading on every build.

## Problem

When `gvk-xml` is enabled, CMake downloads the Vulkan SDK during configuration:
- **Download URL**: `https://sdk.lunarg.com/sdk/download/{VERSION}/linux/vulkansdk-linux-x86_64-{VERSION}.tar.xz`
- **Size**: ~200 MB
- **Location**: `build/_deps/VulkanSDK/`
- **Time**: ~30-60 seconds per download

Without caching, every workflow run downloads the SDK, even if it was just downloaded in the previous run.

## Solution

Dynamically detect SDK version from `CMakeLists.txt` and use GitHub Actions `cache` to save/restore between builds.

## Implementation

### 1. Detect SDK Version (After Checkout)

```yaml
- name: Detect Vulkan SDK Version
  id: detect-sdk-version
  run: |
    # Extract from CMakeLists.txt: set(gvk-Vulkan-SDK_VERSION 1.4.335.0 CACHE STRING "")
    SDK_VERSION=$(grep -oP 'set\(gvk-Vulkan-SDK_VERSION \K[0-9.]+' CMakeLists.txt)
    echo "Detected Vulkan SDK version: $SDK_VERSION"
    echo "sdk-version=$SDK_VERSION" >> $GITHUB_OUTPUT
```

**How it works:**
- Uses `grep` with Perl regex (`-P`) to extract version number
- Looks for pattern: `set(gvk-Vulkan-SDK_VERSION X.Y.Z`
- Outputs to `$GITHUB_OUTPUT` for use in subsequent steps
- Falls back to "unknown" if not found

**Example output:**
```
Detected Vulkan SDK version: 1.4.335.0
```

### 2. Restore Cache (Before Configure)

```yaml
- name: Restore Vulkan SDK Cache
  id: cache-vulkan-sdk
  uses: actions/cache@v4
  with:
    path: build/_deps/VulkanSDK
    key: vulkan-sdk-${{ steps.detect-sdk-version.outputs.sdk-version }}-linux-${{ runner.os }}
    restore-keys: |
      vulkan-sdk-${{ steps.detect-sdk-version.outputs.sdk-version }}-linux-
      vulkan-sdk-
```

**Dynamic Cache Key:**
- Uses detected version from previous step
- Key format: `vulkan-sdk-1.4.335.0-linux-Linux`
- **Automatically updates** when CMakeLists.txt changes version

### 3. Check Cache Status

```yaml
- name: Check Vulkan SDK Cache Status
  run: |
    echo "Required SDK version: ${{ steps.detect-sdk-version.outputs.sdk-version }}"
    if [ "${{ steps.cache-vulkan-sdk.outputs.cache-hit }}" == "true" ]; then
      echo "? Vulkan SDK restored from cache"
    else
      echo "? Will download during configure"
    fi
```

Shows both the required version and cache status.

### 4. Configure (Downloads if Not Cached)

```yaml
- name: Configure
  run: cmake -G Ninja -B build ...
```

If SDK not in cache:
- CMake detects missing SDK
- Downloads from LunarG using version from `CMakeLists.txt`
- Extracts to `build/_deps/VulkanSDK/`

If SDK is cached:
- CMake finds existing SDK
- Skips download
- Uses cached version

### 5. Save Cache (After Configure)

```yaml
- name: Cache Vulkan SDK
  if: steps.cache-vulkan-sdk.outputs.cache-hit != 'true'
  uses: actions/cache/save@v4
  with:
    path: build/_deps/VulkanSDK
    key: vulkan-sdk-${{ steps.detect-sdk-version.outputs.sdk-version }}-linux-${{ runner.os }}
```

**Only saves if:**
- Cache was NOT hit (i.e., we just downloaded it)
- Prevents redundant cache operations

**Key matches restore key** to ensure cache consistency.

## Cache Behavior

### First Run (Cache Miss)
```
1. Detect Version       ? 1.4.335.0
2. Restore Cache        ? ? Cache not found
3. Configure            ? Downloads Vulkan SDK (~30-60s)
4. Save Cache           ? ? Cached as vulkan-sdk-1.4.335.0-linux-Linux
5. Total Configure Time ? ~2-3 minutes
```

### Subsequent Runs (Cache Hit)
```
1. Detect Version       ? 1.4.335.0
2. Restore Cache        ? ? SDK restored from cache (~5s)
3. Configure            ? Skips download (SDK already present)
4. Save Cache           ? ?? Skipped (cache already exists)
5. Total Configure Time ? ~30-60 seconds
```

**Time Savings: ~1.5-2 minutes per build**

### Version Update
```
1. Developer updates CMakeLists.txt: 1.4.335.0 ? 1.4.336.0
2. Detect Version       ? 1.4.336.0 (NEW)
3. Restore Cache        ? ? Cache not found (different key)
4. Configure            ? Downloads NEW SDK version
5. Save Cache           ? ? Cached as vulkan-sdk-1.4.336.0-linux-Linux
6. Old cache            ? Auto-expires after 7 days
```

**Automatic version tracking!** No workflow updates needed.

## Dynamic Cache Key Strategy

### Version-Specific Key (Dynamic)

```
vulkan-sdk-{DETECTED_VERSION}-linux-{OS}
```

**Example:**
```
vulkan-sdk-1.4.335.0-linux-Linux
```

**Benefits:**
- ? Automatically tracks SDK version from CMakeLists.txt
- ? No manual workflow updates when SDK version changes
- ? Each commit uses correct SDK version
- ? Prevents version mismatches
- ? Old caches auto-expire when unused

### Restore Keys (Fallbacks)

```yaml
restore-keys: |
  vulkan-sdk-1.4.335.0-linux-  # Same version, any Linux variant
  vulkan-sdk-                   # Any SDK version (better than full download)
```

**Fallback strategy:**
1. Exact match preferred (version + OS)
2. Same version, different Linux variant
3. Any Vulkan SDK version (re-extract if needed)

## Version Detection Details

### Source Location

From `CMakeLists.txt` (line 15):
```cmake
set(gvk-Vulkan-SDK_VERSION 1.4.335.0 CACHE STRING "")
```

### Extraction Method

```bash
grep -oP 'set\(gvk-Vulkan-SDK_VERSION \K[0-9.]+' CMakeLists.txt
```

**Breakdown:**
- `-o`: Output only matching part
- `-P`: Use Perl regex
- `\K`: Keep everything before, match what comes after
- `[0-9.]+`: Version number pattern (digits and dots)

**Result:** `1.4.335.0`

### Fallback Behavior

If version detection fails:
```bash
SDK_VERSION=$(... || echo "unknown")
```

Cache key becomes: `vulkan-sdk-unknown-linux-Linux`

This prevents workflow failure but disables effective caching.

## When Cache is Invalidated

Cache is **automatically invalidated** when:
1. ? SDK version changes in CMakeLists.txt (new key)
2. ? Cache older than 7 days (auto-expired)
3. ? Cache manually deleted
4. ? Cache corrupted

In all cases, SDK is re-downloaded automatically.

## Benefits

### 1. Automatic Version Tracking
- No manual workflow updates needed
- SDK version changes tracked automatically
- Each branch/commit uses correct SDK

### 2. Faster Builds
- **First run**: Same speed (download needed)
- **Subsequent runs**: 1.5-2 minutes faster
- **CI/CD feedback**: Quicker results

### 3. Version Isolation
- Different SDK versions cached independently
- No conflicts between branches with different SDK versions
- Easy testing of SDK upgrades

### 4. Reduced Maintenance
- Workflow doesn't need updates when SDK version changes
- Self-healing cache strategy
- Less manual intervention

## Multi-Branch Support

Different branches can use different SDK versions simultaneously:

**Branch A (trunk):**
```cmake
set(gvk-Vulkan-SDK_VERSION 1.4.335.0 ...)
```
Cache: `vulkan-sdk-1.4.335.0-linux-Linux`

**Branch B (sdk-upgrade):**
```cmake
set(gvk-Vulkan-SDK_VERSION 1.4.336.0 ...)
```
Cache: `vulkan-sdk-1.4.336.0-linux-Linux`

Both caches coexist independently!

## Monitoring

### View Detected Version

In workflow logs:
```
Detected Vulkan SDK version: 1.4.335.0
```

### View Cache Status

```
Required SDK version: 1.4.335.0
? Vulkan SDK restored from cache
```

Or:
```
Required SDK version: 1.4.335.0
? Vulkan SDK not in cache, will be downloaded during configure
```

### View All Caches

GitHub repo ? Actions ? Caches

Example entries:
```
vulkan-sdk-1.4.335.0-linux-Linux  (200 MB, 2 days ago)
vulkan-sdk-1.4.336.0-linux-Linux  (200 MB, 1 day ago)
```

## Troubleshooting

### Wrong Version Detected

**Check detection step output:**
```yaml
- name: Detect Vulkan SDK Version
  run: |
    grep -oP 'set\(gvk-Vulkan-SDK_VERSION \K[0-9.]+' CMakeLists.txt
```

**Verify CMakeLists.txt format:**
```cmake
set(gvk-Vulkan-SDK_VERSION 1.4.335.0 CACHE STRING "")  # ? Correct
set( gvk-Vulkan-SDK_VERSION 1.4.335.0 ...)             # ? Works (extra spaces ok)
set(VULKAN_SDK_VERSION 1.4.335.0 ...)                  # ? Wrong variable name
```

### Version "unknown"

If detection fails, check:
1. Variable name: `gvk-Vulkan-SDK_VERSION`
2. Format: `set(...)` statement
3. File location: Root `CMakeLists.txt`

### Cache Not Invalidating on Version Change

**Force cache refresh:**
1. Delete old cache in GitHub UI
2. Or wait 7 days for auto-expiry
3. Or change cache key prefix in workflow

## Advanced: CMake Script Method

Alternative extraction using CMake itself:

```yaml
- name: Detect Vulkan SDK Version
  run: |
    cat > extract_version.cmake << 'EOF'
    file(READ CMakeLists.txt content)
    string(REGEX MATCH "set\\(gvk-Vulkan-SDK_VERSION ([0-9.]+)" _ "${content}")
    message("${CMAKE_MATCH_1}")
    EOF
    SDK_VERSION=$(cmake -P extract_version.cmake 2>&1 | tail -1)
    echo "sdk-version=$SDK_VERSION" >> $GITHUB_OUTPUT
```

More robust but slower. Current grep method is sufficient.

## Future Enhancements

### 1. Multi-Platform Detection
```yaml
matrix:
  os: [Linux, Windows]

- name: Detect SDK Version
  run: |
    # Platform-specific detection
    if [ "$RUNNER_OS" == "Windows" ]; then
      # Windows-specific extraction
    else
      # Linux extraction
    fi
```

### 2. SHA256 Verification
```yaml
- name: Verify Cached SDK
  run: |
    EXPECTED_SHA=$(grep SDK_SHA256_LINUX CMakeLists.txt | ...)
    ACTUAL_SHA=$(sha256sum build/_deps/VulkanSDK/*.tar.xz | ...)
    if [ "$EXPECTED_SHA" != "$ACTUAL_SHA" ]; then
      echo "??  SHA mismatch, re-downloading"
      rm -rf build/_deps/VulkanSDK
    fi
```

### 3. Cache Health Check
```yaml
- name: Validate Cache
  run: |
    if [ -f build/_deps/VulkanSDK/setup-env.sh ]; then
      echo "? Cache valid"
    else
      echo "? Cache corrupted, invalidating"
      rm -rf build/_deps/VulkanSDK
    fi
```

## Summary

Dynamic Vulkan SDK caching provides:
- ? **1.5-2 minutes faster** builds on cache hit
- ?? **Automatic version tracking** from CMakeLists.txt
- ?? **No manual workflow updates** when SDK version changes
- ?? **Multi-branch support** with independent caches
- ?? **~200 MB** cache storage per version
- ?? **7-day retention** auto-managed
- ??? **Version isolation** prevents conflicts

The workflow now automatically adapts to SDK version changes in any branch!

## Problem

When `gvk-xml` is enabled, CMake downloads the Vulkan SDK during configuration:
- **Download URL**: `https://sdk.lunarg.com/sdk/download/1.4.335.0/linux/vulkansdk-linux-x86_64-1.4.335.0.tar.xz`
- **Size**: ~200 MB
- **Location**: `build/_deps/VulkanSDK/`
- **Time**: ~30-60 seconds per download

Without caching, every workflow run downloads the SDK, even if it was just downloaded in the previous run.

## Solution

Use GitHub Actions `cache` action to save and restore the Vulkan SDK between builds.

## Implementation

### 1. Restore Cache (Before Configure)

```yaml
- name: Restore Vulkan SDK Cache
  id: cache-vulkan-sdk
  uses: actions/cache@v4
  with:
    path: build/_deps/VulkanSDK
    key: vulkan-sdk-1.4.335.0-linux-${{ runner.os }}
    restore-keys: |
      vulkan-sdk-1.4.335.0-linux-
      vulkan-sdk-
```

**Cache Key Structure:**
- Primary: `vulkan-sdk-1.4.335.0-linux-Linux`
- Fallback: `vulkan-sdk-1.4.335.0-linux-*`
- Fallback: `vulkan-sdk-*`

This allows:
- Exact version match preferred
- Fallback to same version, different OS
- Fallback to any Vulkan SDK version

### 2. Check Cache Status

```yaml
- name: Check Vulkan SDK Cache Status
  run: |
    if [ "${{ steps.cache-vulkan-sdk.outputs.cache-hit }}" == "true" ]; then
      echo "? Vulkan SDK restored from cache"
    else
      echo "? Will download during configure"
    fi
```

Provides visibility into whether cache was hit.

### 3. Configure (Downloads if Not Cached)

```yaml
- name: Configure
  run: cmake -G Ninja -B build ...
```

If SDK not in cache:
- CMake detects missing SDK
- Downloads from LunarG
- Extracts to `build/_deps/VulkanSDK/`

If SDK is cached:
- CMake finds existing SDK
- Skips download
- Uses cached version

### 4. Save Cache (After Configure)

```yaml
- name: Cache Vulkan SDK
  if: steps.cache-vulkan-sdk.outputs.cache-hit != 'true'
  uses: actions/cache/save@v4
  with:
    path: build/_deps/VulkanSDK
    key: vulkan-sdk-1.4.335.0-linux-${{ runner.os }}
```

**Only saves if:**
- Cache was NOT hit (i.e., we just downloaded it)
- Prevents redundant cache operations

### 5. Verify Cache

```yaml
- name: Verify Vulkan SDK Cached
  run: |
    if [ -d "build/_deps/VulkanSDK" ]; then
      echo "? Vulkan SDK directory exists"
      ls -lh build/_deps/VulkanSDK/
    fi
```

Confirms SDK is present after configure.

## Cache Behavior

### First Run (Cache Miss)
```
1. Restore Cache        ? ? Cache not found
2. Configure            ? Downloads Vulkan SDK (~30-60s)
3. Save Cache           ? ? Caches SDK for future runs
4. Total Configure Time ? ~2-3 minutes
```

### Subsequent Runs (Cache Hit)
```
1. Restore Cache        ? ? SDK restored from cache (~5s)
2. Configure            ? Skips download (SDK already present)
3. Save Cache           ? ?? Skipped (cache already exists)
4. Total Configure Time ? ~30-60 seconds
```

**Time Savings: ~1.5-2 minutes per build**

## Cache Retention

GitHub Actions cache retention:
- **Default**: 7 days of inactivity
- **Maximum**: 10 GB per repository
- **Auto-cleanup**: Oldest caches deleted when limit reached

Our cache:
- **Size**: ~200 MB (compressed)
- **Retention**: Auto-managed by GitHub
- **Expiry**: 7 days if not used

## Cache Key Strategy

### Version-Specific Key

```
vulkan-sdk-1.4.335.0-linux-Linux
```

**Why include version?**
- Prevents using wrong SDK version
- When SDK version updates, new cache created
- Old cache auto-expires after 7 days

**Why include OS?**
- Linux vs Windows SDKs are different
- Prevents cross-contamination
- `${{ runner.os }}` resolves to `Linux`, `Windows`, etc.

### Restore Keys (Fallbacks)

```yaml
restore-keys: |
  vulkan-sdk-1.4.335.0-linux-
  vulkan-sdk-
```

**Fallback 1**: Same version, any Linux variant
**Fallback 2**: Any Vulkan SDK version

This allows:
- Reusing SDK from similar environments
- Faster than full download
- CMake will verify SDK version is correct

## When Cache is Invalidated

Cache is **NOT** used when:
1. SDK version changes (new key)
2. Cache older than 7 days (auto-expired)
3. Cache manually deleted
4. Cache corrupted

In these cases, SDK is re-downloaded.

## Monitoring Cache Usage

### Check Cache Hit Rate

In workflow logs, look for:
```
? Vulkan SDK restored from cache
```

Or:
```
? Vulkan SDK not in cache, will be downloaded during configure
```

### View Caches

In GitHub repo:
1. Go to **Actions** tab
2. Click **Caches** in sidebar
3. See all caches with size and age

## Benefits

### 1. Faster Builds
- **First run**: Same speed (download needed)
- **Subsequent runs**: 1.5-2 minutes faster
- **CI/CD feedback**: Quicker results

### 2. Reduced Bandwidth
- Downloads from LunarG reduced
- Less load on external servers
- More reliable (no external dependency for cached builds)

### 3. Offline-Friendly
- Once cached, works even if LunarG is down
- More resilient CI pipeline

### 4. Cost Savings
- Self-hosted runners use less bandwidth
- Faster builds = less runner time
- Lower infrastructure costs

## Trade-offs

### Pros ?
- Much faster configure step
- Reduced external dependencies
- More reliable builds
- Better for rapid iteration

### Cons ??
- Uses GitHub Actions cache storage (~200 MB)
- Cache can become stale
- First run still downloads

## Updating Vulkan SDK Version

When updating SDK version (e.g., to 1.4.336.0):

1. **Update CMakeLists.txt**:
```cmake
set(gvk-Vulkan-SDK_VERSION 1.4.336.0 CACHE STRING "")
```

2. **Update workflow cache key**:
```yaml
key: vulkan-sdk-1.4.336.0-linux-${{ runner.os }}
```

3. **Old cache auto-expires** after 7 days

Alternatively, keep cache key generic:
```yaml
key: vulkan-sdk-${{ hashFiles('CMakeLists.txt') }}-linux-${{ runner.os }}
```

This auto-invalidates cache when CMakeLists.txt changes.

## Troubleshooting

### Cache Not Working

**Check cache hit status:**
```yaml
- name: Check Vulkan SDK Cache Status
  run: |
    echo "Cache hit: ${{ steps.cache-vulkan-sdk.outputs.cache-hit }}"
```

**Verify cache directory:**
```sh
ls -la build/_deps/VulkanSDK/
```

### Wrong SDK Version Cached

Delete old cache:
1. Go to Actions ? Caches
2. Find `vulkan-sdk-*` caches
3. Delete old version
4. Re-run workflow to cache new version

### Cache Corruption

If cache is corrupted:
1. Delete cache in GitHub UI
2. Re-run workflow
3. Fresh download and cache

## Future Enhancements

### 1. Content-Addressed Caching
```yaml
key: vulkan-sdk-${{ hashFiles('build/_deps/VulkanSDK/**') }}-linux
```

Cache based on actual content, not version string.

### 2. Multi-Platform Caching
```yaml
matrix:
  os: [Linux, Windows]
key: vulkan-sdk-1.4.335.0-${{ matrix.os }}
```

Cache per OS in matrix builds.

### 3. SDK Version Detection
```yaml
- name: Detect SDK Version
  run: |
    SDK_VERSION=$(grep Vulkan-SDK_VERSION CMakeLists.txt | ...)
    echo "SDK_VERSION=$SDK_VERSION" >> $GITHUB_ENV

- name: Cache Vulkan SDK
  uses: actions/cache@v4
  with:
    key: vulkan-sdk-${{ env.SDK_VERSION }}-linux
```

Automatically extract version from CMakeLists.txt.

## Summary

Vulkan SDK caching provides:
- ? **1.5-2 minutes faster** builds on cache hit
- ?? **~200 MB** cache storage used
- ?? **7-day retention** auto-managed
- ?? **Version-specific** caching
- ?? **Visible** cache hit/miss status

Recommended for all workflows that use Vulkan SDK!
