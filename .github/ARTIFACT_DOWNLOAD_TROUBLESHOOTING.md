# Artifact Download Failures - Troubleshooting Guide

## Symptom

Godzilla runner shows:
```
Error: Unable to download artifact(s): Unable to download and extract artifact: 
Artifact download failed after 5 retries.
```

## Root Cause

Self-hosted runners need outbound network access to GitHub's artifact storage (Azure Blob Storage):
- **Domain**: `*.blob.core.windows.net`
- **Port**: 443 (HTTPS)
- **Example URL**: `https://productionresultssa16.blob.core.windows.net/...`

The error indicates the godzilla runner cannot reach Azure Blob Storage after 5 retry attempts.

## Common Causes

1. **Firewall/Proxy Blocking**
   - Corporate firewall blocking Azure domains
   - Proxy not configured for runner
   - SSL inspection breaking HTTPS connections

2. **Network Connectivity**
   - Runner in network segment without internet access
   - DNS resolution failing for `*.blob.core.windows.net`
   - Temporary network outage

3. **Runner Configuration**
   - Runner not configured with proxy settings
   - Wrong network interface/routing
   - IPv6 vs IPv4 issues

## Diagnostic Steps

### 1. Check Network Connectivity from Runner

SSH to godzilla runner and test:

```bash
# Test DNS resolution
nslookup productionresultssa16.blob.core.windows.net

# Test HTTPS connectivity
curl -v https://productionresultssa16.blob.core.windows.net/

# Check proxy settings
echo $HTTP_PROXY
echo $HTTPS_PROXY
echo $NO_PROXY

# Test specific artifact URL (copy from workflow logs)
curl -I "https://productionresultssa16.blob.core.windows.net/actions-results/..."
```

### 2. Compare with Working Runner (mage-b580)

Since mage-b580 successfully downloads artifacts, compare:

```bash
# On both runners, check:
traceroute productionresultssa16.blob.core.windows.net
ping productionresultssa16.blob.core.windows.net
curl -I https://github.com/
```

### 3. Check Runner Logs

```bash
# On godzilla runner
cd /home/intel-gvk-ga-runner-1/actions-runner
tail -f _diag/Worker_*.log
```

## Solutions

### Option 1: Configure Proxy (if behind corporate firewall)

Edit runner's `.env` file:
```bash
# On godzilla runner
cd /home/intel-gvk-ga-runner-1/actions-runner
nano .env

# Add:
HTTP_PROXY=http://proxy.company.com:8080
HTTPS_PROXY=http://proxy.company.com:8080
NO_PROXY=localhost,127.0.0.1,.company.com
```

Restart runner:
```bash
sudo ./svc.sh stop
sudo ./svc.sh start
```

### Option 2: Whitelist Azure Blob Storage

Ask network admins to allow outbound HTTPS to:
- `*.blob.core.windows.net` (all Azure regions)
- Or specifically: `productionresultssa16.blob.core.windows.net`

### Option 3: Use Alternative Artifact Transfer

If direct download continues to fail, we can:
1. Upload artifacts to a different location (e.g., shared network drive)
2. Use GitHub cache instead of artifacts
3. Download on one runner and copy to others via internal network

### Option 4: Increase Timeout and Retries

Already added in workflow:
```yaml
- name: Download Test Package
  uses: actions/download-artifact@v4
  timeout-minutes: 10  # Give more time for slow connections
```

## Workaround: Skip Failing Runner Temporarily

If godzilla remains unreachable, temporarily test only on mage-b580:

```yaml
# In .github/workflows/build-and-test-docker.yml
matrix:
  runner:
    # - name: "godzilla (NVIDIA RTX 6000)"  # Commented out
    #   label: nvidia-rtx6000
    #   check_name: "godzilla - NVIDIA RTX 6000"
    - name: "mage-b580 (Intel B580)"
      label: intel-b580
      check_name: "mage-b580 - Intel B580"
```

## Why Mage Works But Godzilla Doesn't

Possible reasons:
1. **Different network segments**: Runners on different VLANs with different firewall rules
2. **Different routing**: Godzilla routes through different gateway
3. **Runner-specific config**: Godzilla missing proxy/network configuration
4. **Timing**: Temporary network issue specific to godzilla's network path

## Verification After Fix

Once network issue is resolved, verify with:

```bash
# On godzilla runner, manually download an artifact
cd /tmp
curl -L -o test.zip "https://productionresultssa16.blob.core.windows.net/actions-results/..." 
unzip test.zip
ls -la
```

Then re-run the workflow to confirm download succeeds.

## Prevention

1. **Document network requirements** for self-hosted runners
2. **Test connectivity** before adding new runners
3. **Monitor runner health** with periodic connectivity checks
4. **Keep runners in same network segment** for consistency

## Current Workflow Changes

Added verification step after download:
```yaml
- name: Verify Test Package Downloaded
  run: |
    if [ ! -f "gvk-test-package/gvk-string.tests" ]; then
      echo "ERROR: Test executable not found!"
      exit 1
    fi
```

This provides clearer error messages if download fails silently.

## Next Steps

1. Contact lab admins about godzilla network connectivity
2. Check if godzilla needs proxy configuration
3. Verify firewall rules for `*.blob.core.windows.net`
4. Consider using internal artifact storage if external access is restricted
