#!/bin/bash

# Thailand DAB+ GUI Builder Script
# Uses Docker to build, extracts binary to run locally

echo "🇹🇭 Thailand DAB+ GUI Builder"
echo "==============================="

# Build using Docker but extract the binary
echo "📦 Building GUI in Docker container..."

# Create a temporary container to build
docker run --rm \
    -v $(pwd):/src/welle.io \
    -w /src/welle.io \
    ubuntu:24.04 \
    bash -c "
        apt-get update -y && \
        apt-get install -y \
            git build-essential cmake \
            qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-charts-dev \
            libfaad-dev libfdk-aac-dev libmpg123-dev librtlsdr-dev libairspy-dev \
            libsoapysdr-dev libsndfile1-dev libasound2-dev libpulse-dev \
            fonts-thai-tlwg locales && \
        locale-gen th_TH.UTF-8 && \
        mkdir -p build && cd build && \
        cmake .. \
            -DBUILD_WELLE_IO=ON \
            -DBUILD_WELLE_CLI=ON \
            -DRTLSDR=ON \
            -DAIRSPY=ON \
            -DSOAPYSDR=ON \
            -DPROFILING=ON \
            -DCMAKE_BUILD_TYPE=Release && \
        make -j\$(nproc) && \
        echo '✅ Build completed successfully' && \
        ls -la src/welle-io/welle-io
    "

if [ $? -eq 0 ]; then
    echo "✅ GUI build successful!"
    echo "📍 Binary location: build/src/welle-io/welle-io"
    echo ""
    echo "🚀 To run Thailand DAB+ GUI locally:"
    echo "   cd welle.io/build/src/welle-io"
    echo "   ./welle-io"
    echo ""
    echo "📡 Default Thailand channel: 12B (225.648 MHz - Bangkok)"
else
    echo "❌ Build failed"
    exit 1
fi
