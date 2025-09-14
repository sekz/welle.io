# 🇹🇭 Thailand DAB+ GUI Setup Instructions

## Current Status

✅ **CLI Docker Service**: **WORKING** - Thailand DAB+ web interface accessible at http://localhost:8080

❌ **GUI Docker Service**: **REQUIRES SETUP** - Qt6 packages needed for local GUI build

## Option 1: Use Working CLI Web Interface (Recommended)

The CLI service is fully functional with Thailand DAB+ compliance:

```bash
# Start Thailand DAB+ web interface
cd welle.io
docker compose up welle-cli-thailand -d

# Access web interface
open http://localhost:8080
```

**Features Available:**
- ✅ Thailand frequency plan (Channel 12B - Bangkok 225.648 MHz)
- ✅ Thai character set support (Profile 0x0E)
- ✅ NBTC compliance checking
- ✅ Web-based spectrum analyzer
- ✅ Service information display
- ✅ Audio streaming capabilities

## Option 2: Install GUI Dependencies (Requires sudo)

To build and run the native GUI application:

### 🚀 Quick Setup (Recommended)

Use the unified Thailand DAB+ GUI builder:

```bash
cd welle.io

# One-command setup (install dependencies + build)
./build-thailand-gui.sh install && ./build-thailand-gui.sh build

# Or step by step:
./build-thailand-gui.sh check     # Check system requirements
./build-thailand-gui.sh install   # Install dependencies (requires sudo)
./build-thailand-gui.sh build     # Build GUI and CLI
```

### Available Commands

```bash
./build-thailand-gui.sh help      # Show all options
./build-thailand-gui.sh check     # Check system requirements
./build-thailand-gui.sh install   # Install Qt6 and dependencies
./build-thailand-gui.sh build     # Build GUI locally
./build-thailand-gui.sh clean     # Clean build directory
./build-thailand-gui.sh build --verbose  # Build with detailed output
```

### Manual Installation (Alternative)

```bash
sudo apt update && sudo apt install -y \
  build-essential cmake \
  qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-charts-dev \
  libfaad-dev libfdk-aac-dev libmpg123-dev \
  librtlsdr-dev libairspy-dev libsoapysdr-dev \
  libasound2-dev libpulse-dev libsndfile1-dev \
  fonts-thai-tlwg
```

### Run Thailand DAB+ GUI

After successful build:

```bash
# GUI application
cd build/src/welle-io && ./welle-io

# CLI application  
cd build && ./welle-cli -c 12B -w 8080
```

## 🇹🇭 Thailand DAB+ Features

### Implemented Features
- ✅ **Thai Character Set (0x0E)**: Full support for Thai text in service labels
- ✅ **NBTC Frequency Plan**: Thailand DAB+ channels (5A-12D, 174-240 MHz)
- ✅ **Bangkok Frequencies**: 225.648 MHz (12B), 227.360 MHz (12C), 229.072 MHz (12D)
- ✅ **Thai Service Parser**: Mixed Thai-English service information
- ✅ **NBTC Compliance**: Regulation compliance checking (ผว. 104-2567)
- ✅ **Thai UI Components**: QML components with Thai font support
- ✅ **Thai Translations**: Full interface localization (th_TH.ts)

### Configuration Files
- `resources/thailand/frequency-plan.json`: Thailand frequency allocations
- `resources/thailand/service-presets.json`: Sample service configurations  
- `resources/thailand/regional-coverage.json`: Regional coverage data

## Hardware Requirements

### RTL-SDR Setup
```bash
# Install RTL-SDR drivers
sudo apt install rtl-sdr

# Test RTL-SDR device
rtl_test -t

# Check USB devices
lsusb | grep RTL
```

### Supported Hardware
- ✅ RTL-SDR dongles (RTL2832U + R820T/R820T2)
- ✅ Airspy receivers  
- ✅ SoapySDR compatible devices
- ✅ Raw IQ file playback

## Usage Examples

### CLI Web Interface
```bash
# Thailand DAB+ web interface
./run-thailand-dab.sh

# Custom channel and port
docker run -p 8080:8080 welle-cli-thailand:latest \
  welle-cli -c 12B -w 8080 -C 1
```

### GUI Application
```bash
# Start GUI with Thailand defaults
./welle-io --channel 12B

# Bangkok specific frequency
./welle-io --frequency 225648000
```

### Testing Without Hardware
```bash
# Use null device for testing
./welle-cli -c 12B -w 8080
# Web interface will show "No valid device" but functionality works
```

## Troubleshooting

### Qt6 Installation Issues
```bash
# Check what's installed
./build-thailand-gui.sh check

# Install missing packages
./build-thailand-gui.sh install

# Force build even with missing dependencies
./build-thailand-gui.sh build --force
```

### Permission Issues
```bash
# RTL-SDR permissions
sudo usermod -a -G plugdev $USER
# Logout and login again

# Build directory permissions
./build-thailand-gui.sh clean
```

### Build Issues
```bash
# Clean and rebuild with verbose output
./build-thailand-gui.sh clean
./build-thailand-gui.sh build --verbose

# Check system requirements
./build-thailand-gui.sh check
```

## Docker Alternative (Advanced)

If you prefer Docker for GUI:

```bash
# Enable X11 forwarding
xhost +local:docker

# Run GUI in container
docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  --device /dev/bus/usb \
  welle-gui-thailand:latest \
  welle-io
```

## 🎯 Recommendation

**For immediate Thailand DAB+ testing**: Use the working CLI web interface at http://localhost:8080

**For full GUI experience**: Install Qt6 packages and build locally using the instructions above

The CLI web interface provides all essential Thailand DAB+ functionality including Thai character support, NBTC compliance, and Bangkok frequency presets.