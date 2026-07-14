
set HTTP_PROXY=http://proxy.placeholder.com:8080
set HTTPS_PROXY=http://proxy.placeholder.com:8080
set NO_PROXY=localhost,127.0.0.0/8,.intel.com
git config --global http.proxy http://proxy.placeholder.com:8080
git config --global https.proxy http://proxy.placeholder.com:8080
git config --global --list
