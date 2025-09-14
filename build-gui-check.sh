#!/bin/bash

# Thailand DAB+ GUI Builder - Package Check
echo "🇹🇭 Thailand DAB+ GUI Builder - System Check"
echo "============================================="

echo "🔍 Checking system requirements..."

# Check for build tools
echo -n "Build tools (cmake, gcc): "
if command -v cmake &> /dev/null && command -v gcc &> /dev/null; then
    echo "✅ Found"
else
    echo "❌ Missing"
    echo "   Install with: sudo apt install build-essential cmake"
fi

# Check for Qt6
echo -n "Qt6 development: "
if dpkg -l | grep -q qt6-base-dev; then
    echo "✅ Found"
    QT6_OK=1
else
    echo "❌ Missing"
    echo "   Install with: sudo apt install qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-charts-dev"
    QT6_OK=0
fi

# Check for audio libraries
echo -n "Audio libraries: "
if dpkg -l | grep -q libfaad-dev && dpkg -l | grep -q libasound2-dev; then
    echo "✅ Found"
    AUDIO_OK=1
else
    echo "❌ Missing"
    echo "   Install with: sudo apt install libfaad-dev libfdk-aac-dev libmpg123-dev libasound2-dev libpulse-dev libsndfile1-dev"
    AUDIO_OK=0
fi

# Check for SDR libraries
echo -n "SDR libraries: "
if dpkg -l | grep -q librtlsdr-dev; then
    echo "✅ Found"
    SDR_OK=1
else
    echo "❌ Missing"
    echo "   Install with: sudo apt install librtlsdr-dev libairspy-dev libsoapysdr-dev"
    SDR_OK=0
fi

# Check for Thai fonts
echo -n "Thai fonts: "
if dpkg -l | grep -q fonts-thai-tlwg; then
    echo "✅ Found"
    FONTS_OK=1
else
    echo "⚠️ Missing (optional)"
    echo "   Install with: sudo apt install fonts-thai-tlwg"
    FONTS_OK=1  # Not critical for building
fi

echo ""
echo "📋 Summary:"
if [ $QT6_OK -eq 1 ] && [ $AUDIO_OK -eq 1 ] && [ $SDR_OK -eq 1 ]; then
    echo "✅ System ready for Thailand DAB+ GUI build!"
    echo ""
    echo "🚀 Run build with:"
    echo "   cd welle.io"
    echo "   mkdir -p build && cd build"
    echo "   cmake .. -DBUILD_WELLE_IO=ON -DBUILD_WELLE_CLI=ON -DRTLSDR=ON -DAIRSPY=ON -DSOAPYSDR=ON"
    echo "   make -j\$(nproc)"
    echo ""
    echo "🇹🇭 Thailand DAB+ GUI will be at: build/src/welle-io/welle-io"

    # Try to build automatically if all requirements are met
    echo ""
    echo "🔨 Attempting automatic build..."
    cd /home/seksan/workspace/dabplusclient/welle.io
    rm -rf build
    mkdir -p build && cd build

    echo "⚙️ Configuring..."
    if cmake .. -DBUILD_WELLE_IO=ON -DBUILD_WELLE_CLI=ON -DRTLSDR=ON -DAIRSPY=ON -DSOAPYSDR=ON -DCMAKE_BUILD_TYPE=Release; then
        echo "🚀 Building (this may take several minutes)..."
        if make -j$(nproc); then
            echo ""
            echo "🎉 BUILD SUCCESSFUL!"
            echo "📍 GUI Binary: $(pwd)/src/welle-io/welle-io"
            ls -la src/welle-io/welle-io 2>/dev/null || echo "⚠️ GUI binary not found"
        else
            echo "❌ Build failed during compilation"
        fi
    else
        echo "❌ Build failed during configuration"
    fi
else
    echo "❌ Missing required packages. Install them first:"
    echo ""
    echo "sudo apt update && sudo apt install -y \\"
    echo "  build-essential cmake \\"
    echo "  qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-charts-dev \\"
    echo "  libfaad-dev libfdk-aac-dev libmpg123-dev \\"
    echo "  librtlsdr-dev libairspy-dev libsoapysdr-dev \\"
    echo "  libasound2-dev libpulse-dev libsndfile1-dev \\"
    echo "  fonts-thai-tlwg"
fi
