#!/bin/bash
# Quick setup script for Linux self-hosted runner with Docker

set -e

echo "======================================"
echo "GVK Build Environment Setup"
echo "======================================"
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "? Docker is not installed!"
    echo ""
    echo "Install Docker with:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y docker.io"
    echo "  sudo systemctl start docker"
    echo "  sudo systemctl enable docker"
    echo "  sudo usermod -aG docker \$USER"
    echo ""
    echo "Then log out and back in (or restart runner service)"
    exit 1
fi

echo "? Docker is installed"
docker --version
echo ""

# Check if user is in docker group
if ! groups | grep -q docker; then
    echo "??  Warning: Current user is not in docker group"
    echo "   Run: sudo usermod -aG docker $USER"
    echo "   Then log out and back in"
    echo ""
fi

# Check if Docker daemon is running
if ! docker info &> /dev/null; then
    echo "? Docker daemon is not running!"
    echo "   Start it with: sudo systemctl start docker"
    exit 1
fi

echo "? Docker daemon is running"
echo ""

# Build the Docker image
echo "Building gvk-build Docker image..."
echo "This may take several minutes on first run..."
echo ""

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/../.." && pwd )"

cd "$PROJECT_ROOT"

docker build -f .github/workflows/Dockerfile.build -t gvk-build:latest .

echo ""
echo "======================================"
echo "? Setup Complete!"
echo "======================================"
echo ""
echo "Docker image 'gvk-build:latest' is ready."
echo ""
echo "You can now:"
echo "1. Trigger the workflow manually via GitHub UI"
echo "2. Or test locally with:"
echo "   docker run --rm -v \$(pwd):/workspace -w /workspace gvk-build:latest bash"
echo ""
echo "Note: The 'build-linux-docker' job automatically rebuilds"
echo "      this image, so manual rebuilds are only needed for"
echo "      the 'build-linux' job (container-based build)."
echo ""
