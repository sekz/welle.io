#!/bin/bash
# Docker build script for welle.io Thailand DAB+ support

set -e

echo "🇹🇭 Building welle.io Thailand DAB+ Docker Images"
echo "================================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [[ ! -f "Dockerfile.thailand" ]]; then
    print_error "Dockerfile.thailand not found. Please run from welle.io directory."
    exit 1
fi

# Build arguments
BUILD_ARGS=""
if [[ "$1" == "--no-cache" ]]; then
    BUILD_ARGS="--no-cache"
    print_warning "Building with --no-cache (slower but fresh build)"
fi

# 1. Build Thailand DAB+ image
print_status "Building welle.io Thailand DAB+ image..."
docker build ${BUILD_ARGS} -f Dockerfile.thailand -t welle-io-thailand:latest .
if [[ $? -eq 0 ]]; then
    print_status "✅ Thailand DAB+ image built successfully"
else
    print_error "❌ Failed to build Thailand DAB+ image"
    exit 1
fi

# 2. Build original welle-cli image
print_status "Building original welle-cli image..."
docker build ${BUILD_ARGS} -t welle-cli:latest .
if [[ $? -eq 0 ]]; then
    print_status "✅ Original welle-cli image built successfully"
else
    print_warning "⚠️  Original welle-cli build failed (non-critical)"
fi

# 3. List built images
print_status "Built Docker images:"
docker images | grep -E "(welle|thailand)" | head -10

echo ""
print_status "🎉 Build completed!"
echo ""
print_status "Available commands:"
echo "  🖥️  CLI mode:    docker-compose up welle-cli-thailand"
echo "  🖼️  GUI mode:    docker-compose --profile gui up welle-gui-thailand"
echo "  🛠️  Dev mode:    docker-compose --profile dev up welle-thailand-dev"
echo "  🧪 Run tests:   docker run --rm welle-io-thailand:latest test_thailand_compliance"
echo ""
print_status "For RTL-SDR support, make sure your USB device is connected!"
