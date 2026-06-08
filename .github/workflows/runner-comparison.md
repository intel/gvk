# Runner Comparison Report

**Generated:** 2026-06-08 09:07:50

---

## Runners Detected

- **godzilla (NVIDIA RTX 6000):** yes
- **mage-b580 (Intel B580):** yes

---

## Quick Comparison

| Feature | godzilla | mage-b580 |
|---------|----------|-----------|
| Docker | Docker version 29.2.1, build a5c7197 | Docker version 28.5.2, build ecc6942 |
| GPU | ASPEED Technology, Inc. ASPEED Graphics Family (rev 30) | Intel Corporation Device e20b |

---

## Full Reports

# System Report: godzilla

**Generated:** 2026-06-08 09:07:38
**Runner Labels:** self-hosted, Linux, X64, nvidia-rtx6000

---

## Runner Information

```
Runner Name: godzilla
Runner OS: Linux
Runner Architecture: X64
Hostname: godzilla
User: intel-gvk-ga-runner-1
```

---

## Operating System

```
NAME="Red Hat Enterprise Linux"
VERSION="8.10 (Ootpa)"
ID="rhel"
ID_LIKE="fedora"
VERSION_ID="8.10"
PLATFORM_ID="platform:el8"
PRETTY_NAME="Red Hat Enterprise Linux 8.10 (Ootpa)"
ANSI_COLOR="0;31"
CPE_NAME="cpe:/o:redhat:enterprise_linux:8::baseos"
HOME_URL="https://www.redhat.com/"
DOCUMENTATION_URL="https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8"
BUG_REPORT_URL="https://issues.redhat.com/"

REDHAT_BUGZILLA_PRODUCT="Red Hat Enterprise Linux 8"
REDHAT_BUGZILLA_PRODUCT_VERSION=8.10
REDHAT_SUPPORT_PRODUCT="Red Hat Enterprise Linux"
REDHAT_SUPPORT_PRODUCT_VERSION="8.10"
```

**Kernel:** `4.18.0-553.16.1.el8_10.x86_64`
**Architecture:** `x86_64`

---

## Hardware

### CPU
```
Architecture:        x86_64
CPU(s):              56
Thread(s) per core:  2
Core(s) per socket:  14
Socket(s):           2
Vendor ID:           GenuineIntel
Model name:          Intel(R) Xeon(R) CPU E5-2680 v4 @ 2.40GHz
NUMA node0 CPU(s):   0-13,28-41
NUMA node1 CPU(s):   14-27,42-55
```

### Memory
```
              total        used        free      shared  buff/cache   available
Mem:          125Gi       8.3Gi        94Gi        56Mi        22Gi       115Gi
Swap:         127Gi          0B       127Gi
```

### Disk Space
```
Filesystem                               Size  Used Avail Use% Mounted on
tmpfs                                     63G     0   63G   0% /dev/shm
/dev/mapper/rhelb-root                   3.6T  756G  2.8T  22% /
/dev/sda2                                5.4G  1.2G  4.3G  21% /boot
/dev/sda1                                599M  5.9M  593M   1% /boot/efi
```

---

## GPU Information

### PCI Devices
```
05:00.0 VGA compatible controller: ASPEED Technology, Inc. ASPEED Graphics Family (rev 30)
83:00.0 3D controller: NVIDIA Corporation Device 2bb5 (rev a1)
```

### GPU Driver Modules
```
nvidia_uvm           1736704  4
nvidia_drm            118784  2
nvidia_modeset       1732608  2 nvidia_drm
nvidia              14663680  42 nvidia_uvm,nvidia_modeset
video                  57344  1 nvidia_modeset
drm_kms_helper        184320  5 ast,nvidia_drm
drm                   602112  9 drm_kms_helper,ast,drm_shmem_helper,nvidia,nvidia_drm
```

### NVIDIA Info
```
Mon Jun  8 09:07:38 2026       
+-----------------------------------------------------------------------------------------+
| NVIDIA-SMI 595.58.03              Driver Version: 595.58.03      CUDA Version: 13.2     |
+-----------------------------------------+------------------------+----------------------+
| GPU  Name                 Persistence-M | Bus-Id          Disp.A | Volatile Uncorr. ECC |
| Fan  Temp   Perf          Pwr:Usage/Cap |           Memory-Usage | GPU-Util  Compute M. |
|                                         |                        |               MIG M. |
|=========================================+========================+======================|
|   0  NVIDIA RTX PRO 6000 Blac...    Off |   00000000:83:00.0 Off |                    0 |
| N/A   45C    P0             88W /  600W |      36MiB /  97887MiB |      0%      Default |
|                                         |                        |             Disabled |
+-----------------------------------------+------------------------+----------------------+

+-----------------------------------------------------------------------------------------+
| Processes:                                                                              |
|  GPU   GI   CI              PID   Type   Process name                        GPU Memory |
|        ID   ID                                                               Usage      |
|=========================================================================================|
|    0   N/A  N/A            8675      G   /usr/libexec/Xorg                         4MiB |
|    0   N/A  N/A         2822996      C   ./clustalw2                              14MiB |
+-----------------------------------------------------------------------------------------+
```

---

## Docker

### Docker Version
```
Docker version 29.2.1, build a5c7197
```

### Docker Info
```
Client: Docker Engine - Community
 Version:    29.2.1
 Context:    default
 Debug Mode: false
 Plugins:
  buildx: Docker Buildx (Docker Inc.)
    Version:  v0.31.1
    Path:     /usr/libexec/docker/cli-plugins/docker-buildx
  compose: Docker Compose (Docker Inc.)
    Version:  v5.0.2
    Path:     /usr/libexec/docker/cli-plugins/docker-compose
  scan: Docker Scan (Docker Inc.)
    Version:  v0.23.0
    Path:     /usr/libexec/docker/cli-plugins/docker-scan

Server:
permission denied while trying to connect to the docker API at unix:///var/run/docker.sock
```

---

## Build Tools

### CMake
```
cmake version 3.26.5

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

### Ninja
```
/home/intel-gvk-ga-runner-1/actions-runner/_work/_temp/187f764d-5c85-4ade-a07a-ad8c7f847e97.sh: line 106: ninja: command not found
Ninja not installed
```

### GCC/G++
```
gcc (GCC) 8.5.0 20210514 (Red Hat 8.5.0-22)
g++ (GCC) 8.5.0 20210514 (Red Hat 8.5.0-22)
```

### Clang/Clang++
```
clang version 17.0.6 (Red Hat 17.0.6-1.module+el8.10.0+20808+e12784c0)
clang version 17.0.6 (Red Hat 17.0.6-1.module+el8.10.0+20808+e12784c0)
```

### Vulkan Tools
```
/home/intel-gvk-ga-runner-1/actions-runner/_work/_temp/187f764d-5c85-4ade-a07a-ad8c7f847e97.sh: line 126: vulkaninfo: command not found
vulkaninfo not available
```


---

# System Report: mage-b580

**Generated:** 2026-06-08 09:07:32
**Runner Labels:** self-hosted, Linux, X64, intel-b580

---

## Runner Information

```
Runner Name: mage-b580
Runner OS: Linux
Runner Architecture: X64
Hostname: mage
User: intel-gvk-ga-runner-1
```

---

## Operating System

```
PRETTY_NAME="Ubuntu 24.04.3 LTS"
NAME="Ubuntu"
VERSION_ID="24.04"
VERSION="24.04.3 LTS (Noble Numbat)"
VERSION_CODENAME=noble
ID=ubuntu
ID_LIKE=debian
HOME_URL="https://www.ubuntu.com/"
SUPPORT_URL="https://help.ubuntu.com/"
BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
UBUNTU_CODENAME=noble
LOGO=ubuntu-logo
```

**Kernel:** `6.14.0-37-generic`
**Architecture:** `x86_64`

---

## Hardware

### CPU
```
Architecture:                            x86_64
CPU(s):                                  20
Vendor ID:                               GenuineIntel
Model name:                              Intel(R) Core(TM) Ultra 7 265
Thread(s) per core:                      1
Core(s) per socket:                      20
Socket(s):                               1
NUMA node0 CPU(s):                       0-19
```

### Memory
```
               total        used        free      shared  buff/cache   available
Mem:            30Gi       2.2Gi       6.3Gi       141Mi        23Gi        28Gi
Swap:          2.0Gi       174Mi       1.8Gi
```

### Disk Space
```
Filesystem      Size  Used Avail Use% Mounted on
/dev/nvme0n1p2  1.5T  247G  1.2T  18% /
tmpfs            16G     0   16G   0% /dev/shm
/dev/nvme0n1p1  511M  6.2M  505M   2% /boot/efi
```

---

## GPU Information

### PCI Devices
```
03:00.0 VGA compatible controller: Intel Corporation Device e20b
80:14.5 Non-VGA unclassified device: Intel Corporation Device 7f2f (rev 10)
```

### GPU Driver Modules
```
xe                   3440640  13
drm_gpuvm              45056  1 xe
gpu_sched              61440  1 xe
drm_buddy              24576  1 xe
drm_ttm_helper         16384  1 xe
ttm                   118784  2 drm_ttm_helper,xe
drm_exec               12288  2 drm_gpuvm,xe
drm_suballoc_helper    20480  1 xe
drm_display_helper    278528  1 xe
cec                    94208  2 drm_display_helper,xe
i2c_algo_bit           16384  1 xe
intel_vsec             20480  2 intel_pmc_core,xe
video                  77824  1 xe
```

### NVIDIA Info
```
/home/intel-gvk-ga-runner-1/actions-runner/_work/_temp/058032b3-fe55-4857-8bdf-8894e899e47c.sh: line 74: nvidia-smi: command not found
nvidia-smi not available
```

---

## Docker

### Docker Version
```
Docker version 28.5.2, build ecc6942
```

### Docker Info
```
Client: Docker Engine - Community
 Version:    28.5.2
 Context:    default
 Debug Mode: false
 Plugins:
  buildx: Docker Buildx (Docker Inc.)
    Version:  v0.29.1
    Path:     /usr/libexec/docker/cli-plugins/docker-buildx
  compose: Docker Compose (Docker Inc.)
    Version:  v2.40.3
    Path:     /usr/libexec/docker/cli-plugins/docker-compose

Server:
permission denied while trying to connect to the Docker daemon socket at unix:///var/run/docker.sock: Get "http://%2Fvar%2Frun%2Fdocker.sock/v1.51/info": dial unix /var/run/docker.sock: connect: permission denied
```

---

## Build Tools

### CMake
```
cmake version 3.28.3

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

### Ninja
```
1.13.1
```

### GCC/G++
```
gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
```

### Clang/Clang++
```
/home/intel-gvk-ga-runner-1/actions-runner/_work/_temp/058032b3-fe55-4857-8bdf-8894e899e47c.sh: line 119: clang: command not found
/home/intel-gvk-ga-runner-1/actions-runner/_work/_temp/058032b3-fe55-4857-8bdf-8894e899e47c.sh: line 120: clang++: command not found
```

### Vulkan Tools
```
/home/intel-gvk-ga-runner-1/actions-runner/_work/_temp/058032b3-fe55-4857-8bdf-8894e899e47c.sh: line 126: vulkaninfo: command not found
vulkaninfo not available
```


---

## Recommendations

??  **Docker versions differ** - May need to handle differences

?? **Testing Strategy:**
- Build in Docker on any available runner
- Test on both godzilla (NVIDIA) and mage-b580 (Intel)
- This will catch GPU/driver-specific Vulkan issues
