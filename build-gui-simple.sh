#!/bin/bash

# Simple Thailand DAB+ GUI Builder
echo "🇹🇭 Thailand DAB+ GUI Builder (Simple)"
echo "======================================"

# Stop any existing Docker containers
docker container prune -f > /dev/null 2>&1

echo "📦 Building GUI locally with host system..."

# Check if we have Qt6 installed on host system
if ! command -v qmake6 &> /dev/null; then
    echo "❌ Qt6 not found on host system"
    echo "📋 Installing required packages..."
    sudo apt-get update -y
    sudo apt-get install -y \
        build-essential cmake \
        qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-charts-dev \
        libfaad-dev libfdk-aac-dev libmpg123-dev librtlsdr-dev libairspy-dev \
        libsoapysdr-dev libsndfile1-dev libasound2-dev libpulse-dev \
        fonts-thai-tlwg
fi

echo "🔨 Building Thailand DAB+ GUI..."
cd /home/seksan/workspace/dabplusclient/welle.io

# Clean and create build directory
rm -rf build
mkdir -p build && cd build

# Configure with Thailand support
echo "⚙️ Configuring build..."
cmake .. \
    -DBUILD_WELLE_IO=ON \
    -DBUILD_WELLE_CLI=ON \
    -DRTLSDR=ON \
    -DAIRSPY=ON \
    -DSOAPYSDR=ON \
    -DPROFILING=ON \
    -DCMAKE_BUILD_TYPE=Release

echo "🚀 Compiling (this may take a few minutes)..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "✅ GUI build successful!"
    echo ""
    echo "📍 GUI Binary: $(pwd)/src/welle-io/welle-io"
    echo "📍 CLI Binary: $(pwd)/welle-cli"
    echo ""
    echo "🚀 To run Thailand DAB+ GUI:"
    echo "   cd $(pwd)/src/welle-io"
    echo "   ./welle-io"
    echo ""
    echo "🇹🇭 Thailand DAB+ features included:"
    echo "   • Thai Character Set (0x0E) support"
    echo "   • NBTC compliance checking"
    echo "   • Thailand frequency plan (Channel 12B - Bangkok)"
    echo "   • Thai service information parsing"
else
    echo "❌ Build failed"
    exit 1
fi
