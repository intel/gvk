
#!/bin/bash

export http_proxy=http://proxy.placeholder.com:8080
export https_proxy=http://proxy.placeholder.com:8080
export no_proxy=localhost,127.0.0.0/8,.placeholder.com
git config --global http.proxy http://proxy.placeholder.com:8080
git config --global https.proxy http://proxy.placeholder.com:8080
git config --global --list
