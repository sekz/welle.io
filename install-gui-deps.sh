#!/bin/bash

# Thailand DAB+ GUI Dependencies Installer
echo "🇹🇭 Thailand DAB+ GUI Dependencies Installer"
echo "============================================"

echo "📋 Installing required packages for Thailand DAB+ GUI..."
echo ""
echo "This will install:"
echo "  • Qt6 development packages (GUI framework)"
echo "  • Audio codec libraries (FAAD, FDK-AAC, MPG123)"
echo "  • SDR hardware support (RTL-SDR, AirSpy, SoapySDR)"
echo "  • Audio system libraries (ALSA, PulseAudio)"
echo "  • Thai font support"
echo ""

read -p "Continue with installation? [y/N]: " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Installation cancelled."
    exit 1
fi

echo "🔄 Updating package list..."
sudo apt update

echo "📦 Installing Thailand DAB+ GUI dependencies..."
sudo apt install -y \
    build-essential cmake \
    qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-charts-dev \
    libfaad-dev libfdk-aac-dev libmpg123-dev \
    librtlsdr-dev libairspy-dev libsoapysdr-dev \
    libasound2-dev libpulse-dev libsndfile1-dev \
    fonts-thai-tlwg

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Installation successful!"
    echo ""
    echo "🚀 Now you can build the Thailand DAB+ GUI:"
    echo "   ./build-gui-local.sh"
    echo ""
    echo "📡 Or run the working CLI web interface:"
    echo "   docker compose up welle-cli-thailand -d"
    echo "   # Then open http://localhost:8080"
else
    echo ""
    echo "❌ Installation failed!"
    echo "Please check the error messages above and try again."
    exit 1
fi
