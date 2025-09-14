#!/bin/bash

# Thailand DAB+ GUI Builder Script
# Builds locally on the host system

echo "🇹🇭 Thailand DAB+ GUI Builder (Local Host)"
echo "==========================================="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: Please run this script from the welle.io directory"
    echo "   cd welle.io && ./build-gui-local.sh"
    exit 1
fi

echo "🔍 Checking system requirements..."

# Check for Qt6 development packages
echo "🔍 Checking for Qt6 development packages..."
if ! dpkg -l 2>/dev/null | grep -q "^ii.*qt6-base-dev "; then
    echo "❌ Qt6 development packages not found"
    echo ""
    echo "📋 Required packages need to be installed:"
    echo ""
    echo "sudo apt update && sudo apt install -y \\"
    echo "  build-essential cmake \\"
    echo "  qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-charts-dev \\"
    echo "  libfaad-dev libfdk-aac-dev libmpg123-dev \\"
    echo "  librtlsdr-dev libairspy-dev libsoapysdr-dev \\"
    echo "  libasound2-dev libpulse-dev libsndfile1-dev \\"
    echo "  fonts-thai-tlwg"
    echo ""
    echo "📝 What's currently installed:"
    dpkg -l 2>/dev/null | grep -i qt6 | grep -E "(dev|base)" | head -5 || echo "   No Qt6 development packages found"
    echo ""
    echo "After installing the packages, run this script again:"
    echo "   ./build-gui-local.sh"
    exit 1
fi

# Additional check for Qt6Config.cmake
if ! find /usr -name "Qt6Config.cmake" 2>/dev/null | head -1 | grep -q .; then
    echo "❌ Qt6Config.cmake not found"
    echo "📋 Qt6 development files missing. Install with:"
    echo "   sudo apt install qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-charts-dev"
    exit 1
fi

echo "✅ Qt6 development packages found"
echo "🔨 Building Thailand DAB+ GUI locally..."

# Clean and create build directory
if [ -d "build" ]; then
    echo "🧹 Cleaning existing build directory..."
    # Handle permission issues from Docker builds
    if ! rm -rf build 2>/dev/null; then
        echo "⚠️ Permission issues with build directory, trying alternative cleanup..."
        mv build build.bak.$(date +%s) 2>/dev/null || {
            echo "❌ Cannot clean build directory. Please run:"
            echo "   sudo rm -rf build"
            echo "   Then run this script again."
            exit 1
        }
    fi
fi

mkdir -p build && cd build

echo "⚙️ Configuring build..."
if cmake .. \
    -DBUILD_WELLE_IO=ON \
    -DBUILD_WELLE_CLI=ON \
    -DRTLSDR=ON \
    -DAIRSPY=ON \
    -DSOAPYSDR=ON \
    -DPROFILING=ON \
    -DCMAKE_BUILD_TYPE=Release; then

    echo "🚀 Compiling (this may take several minutes)..."
    if make -j$(nproc); then
        echo ""
        echo "🎉 BUILD SUCCESSFUL!"
        echo "📍 GUI Binary: $(pwd)/src/welle-io/welle-io"
        echo "📍 CLI Binary: $(pwd)/welle-cli"

        # Verify binaries exist
        if [ -f "src/welle-io/welle-io" ]; then
            echo "✅ GUI binary confirmed"
            echo "🚀 To run Thailand DAB+ GUI:"
            echo "   cd $(pwd)/src/welle-io && ./welle-io"
        else
            echo "⚠️ GUI binary not found - check build output above"
        fi

        if [ -f "welle-cli" ]; then
            echo "✅ CLI binary confirmed"
            echo "🌐 To run CLI web interface:"
            echo "   cd $(pwd) && ./welle-cli -c 12B -w 8080"
        fi

        echo ""
        echo "🇹🇭 Thailand DAB+ Features Available:"
        echo "   • Thai Character Set (0x0E) support"
        echo "   • NBTC compliance checking"
        echo "   • Bangkok frequencies (225.648 MHz, 227.360 MHz, 229.072 MHz)"
        echo "   • Thai service information parsing"
        echo "   • Regional coverage support"

    else
        echo "❌ Compilation failed"
        echo "Check the error messages above for details"
        exit 1
    fi
else
    echo "❌ Configuration failed"
    echo "Check that all dependencies are installed correctly"
    exit 1
fi
