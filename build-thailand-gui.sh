#!/bin/bash

# 🇹🇭 Thailand DAB+ GUI Builder - Unified Script
# Combines installation, checking, and building functionality

set -e

SCRIPT_NAME="build-thailand-gui.sh"
VERSION="1.0"

show_help() {
    echo "🇹🇭 Thailand DAB+ GUI Builder v${VERSION}"
    echo "============================================"
    echo ""
    echo "Usage: ./${SCRIPT_NAME} [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  install     Install required GUI dependencies (requires sudo)"
    echo "  check       Check system requirements and dependencies"
    echo "  build       Build Thailand DAB+ GUI locally"
    echo "  docker      Build GUI using Docker container (experimental)"
    echo "  clean       Clean build directory"
    echo "  help        Show this help message"
    echo ""
    echo "Options:"
    echo "  --force     Force build even if dependencies missing"
    echo "  --verbose   Show detailed build output"
    echo "  --quiet     Minimize output"
    echo ""
    echo "Examples:"
    echo "  ./${SCRIPT_NAME} install          # Install all dependencies"
    echo "  ./${SCRIPT_NAME} check            # Check what's installed"
    echo "  ./${SCRIPT_NAME} build            # Build the GUI"
    echo "  ./${SCRIPT_NAME} clean            # Clean build directory"
    echo "  ./${SCRIPT_NAME} build --verbose  # Build with detailed output"
    echo ""
    echo "🚀 Quick Setup:"
    echo "  ./${SCRIPT_NAME} install && ./${SCRIPT_NAME} build"
    echo ""
    echo "🌐 Alternative: Use working CLI web interface:"
    echo "  docker compose up welle-cli-thailand -d"
    echo "  # Then open http://localhost:8080"
}

log() {
    if [ "$QUIET" != "1" ]; then
        echo "$@"
    fi
}

verbose() {
    if [ "$VERBOSE" = "1" ]; then
        echo "🔍 $@"
    fi
}

error() {
    echo "❌ Error: $@" >&2
    exit 1
}

check_directory() {
    if [ ! -f "CMakeLists.txt" ]; then
        error "Please run this script from the welle.io directory"
    fi
}

install_dependencies() {
    log "🇹🇭 Installing Thailand DAB+ GUI Dependencies"
    log "============================================="
    log ""
    log "📋 This will install:"
    log "  • Qt6 development packages (GUI framework)"
    log "  • Audio codec libraries (FAAD, FDK-AAC, MPG123)"
    log "  • SDR hardware support (RTL-SDR, AirSpy, SoapySDR)"
    log "  • Audio system libraries (ALSA, PulseAudio)"
    log "  • Thai font support"
    log ""

    if [ "$FORCE" != "1" ]; then
        read -p "Continue with installation? [y/N]: " -n 1 -r
        echo ""
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            log "Installation cancelled."
            exit 0
        fi
    fi

    verbose "Updating package list..."
    sudo apt update

    log "📦 Installing dependencies..."
    sudo apt install -y \
        build-essential cmake \
        qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-charts-dev \
        libfaad-dev libfdk-aac-dev libmpg123-dev \
        librtlsdr-dev libairspy-dev libsoapysdr-dev \
        libasound2-dev libpulse-dev libsndfile1-dev \
        fonts-thai-tlwg

    if [ $? -eq 0 ]; then
        log ""
        log "✅ Installation successful!"
        log ""
        log "🚀 Next step: Build the Thailand DAB+ GUI"
        log "   ./${SCRIPT_NAME} build"
    else
        error "Installation failed! Check error messages above."
    fi
}

check_dependencies() {
    log "🇹🇭 Thailand DAB+ GUI System Check"
    log "=================================="
    log ""
    log "🔍 Checking system requirements..."

    # Check for build tools
    log -n "Build tools (cmake, gcc): "
    if command -v cmake &> /dev/null && command -v gcc &> /dev/null; then
        log "✅ Found"
        BUILD_TOOLS_OK=1
    else
        log "❌ Missing"
        log "   Install with: sudo apt install build-essential cmake"
        BUILD_TOOLS_OK=0
    fi

    # Check for Qt6 development packages
    log -n "Qt6 development packages: "
    if dpkg -l 2>/dev/null | grep -q "^ii.*qt6-base-dev "; then
        log "✅ Found"
        QT6_OK=1
    else
        log "❌ Missing"
        log "   Install with: ./${SCRIPT_NAME} install"
        QT6_OK=0
    fi

    # Additional check for Qt6Config.cmake
    if [ "$QT6_OK" = "1" ]; then
        if ! find /usr -name "Qt6Config.cmake" 2>/dev/null | head -1 | grep -q .; then
            log "⚠️  Qt6Config.cmake not found (may cause build issues)"
            QT6_OK=0
        fi
    fi

    # Check for audio libraries
    log -n "Audio libraries: "
    if dpkg -l 2>/dev/null | grep -q libfaad-dev && dpkg -l 2>/dev/null | grep -q libasound2-dev; then
        log "✅ Found"
        AUDIO_OK=1
    else
        log "❌ Missing"
        AUDIO_OK=0
    fi

    # Check for SDR libraries
    log -n "SDR libraries: "
    if dpkg -l 2>/dev/null | grep -q librtlsdr-dev; then
        log "✅ Found"
        SDR_OK=1
    else
        log "❌ Missing"
        SDR_OK=0
    fi

    # Check for Thai fonts
    log -n "Thai fonts: "
    if dpkg -l 2>/dev/null | grep -q fonts-thai-tlwg; then
        log "✅ Found"
        FONTS_OK=1
    else
        log "⚠️  Missing (optional)"
        FONTS_OK=1  # Not critical for building
    fi

    log ""
    log "📋 Summary:"
    if [ $BUILD_TOOLS_OK -eq 1 ] && [ $QT6_OK -eq 1 ] && [ $AUDIO_OK -eq 1 ] && [ $SDR_OK -eq 1 ]; then
        log "✅ System ready for Thailand DAB+ GUI build!"
        log ""
        log "🚀 Build with:"
        log "   ./${SCRIPT_NAME} build"
        return 0
    else
        log "❌ Missing required packages."
        log ""
        log "📋 Install missing packages with:"
        log "   ./${SCRIPT_NAME} install"
        log ""
        if [ "$VERBOSE" = "1" ]; then
            log "📝 Currently installed Qt6 packages:"
            dpkg -l 2>/dev/null | grep -i qt6 | grep -E "(dev|base)" | head -5 || log "   No Qt6 development packages found"
        fi
        return 1
    fi
}

clean_build() {
    log "🧹 Cleaning build directory..."

    if [ -d "build" ]; then
        # Handle permission issues from Docker builds
        if ! rm -rf build 2>/dev/null; then
            verbose "Permission issues detected, trying alternative cleanup..."
            if mv build build.bak.$(date +%s) 2>/dev/null; then
                log "✅ Old build directory moved to backup"
            else
                error "Cannot clean build directory. Please run: sudo rm -rf build"
            fi
        else
            log "✅ Build directory cleaned"
        fi
    else
        log "✅ Build directory already clean"
    fi
}

build_gui() {
    log "🇹🇭 Building Thailand DAB+ GUI (Local Host)"
    log "==========================================="

    check_directory

    if [ "$FORCE" != "1" ]; then
        # Check dependencies unless forced
        if ! check_dependencies > /dev/null 2>&1; then
            log ""
            log "⚠️  Dependencies missing. Install them first:"
            log "   ./${SCRIPT_NAME} install"
            log ""
            log "Or force build with: ./${SCRIPT_NAME} build --force"
            exit 1
        fi
    fi

    verbose "Cleaning build directory..."
    clean_build

    mkdir -p build && cd build

    log "⚙️  Configuring build..."
    CMAKE_CMD="cmake .. \
        -DBUILD_WELLE_IO=ON \
        -DBUILD_WELLE_CLI=ON \
        -DRTLSDR=ON \
        -DAIRSPY=ON \
        -DSOAPYSDR=ON \
        -DPROFILING=ON \
        -DCMAKE_BUILD_TYPE=Release"

    if [ "$VERBOSE" = "1" ]; then
        verbose "CMake command: $CMAKE_CMD"
        $CMAKE_CMD
    else
        $CMAKE_CMD > /dev/null
    fi

    if [ $? -ne 0 ]; then
        error "Configuration failed! Run with --verbose for details"
    fi

    log "🚀 Compiling (this may take several minutes)..."
    MAKE_CMD="make -j$(nproc)"

    if [ "$VERBOSE" = "1" ]; then
        $MAKE_CMD
    else
        $MAKE_CMD > /dev/null 2>&1
    fi

    if [ $? -eq 0 ]; then
        log ""
        log "🎉 BUILD SUCCESSFUL!"
        log "📍 GUI Binary: $(pwd)/src/welle-io/welle-io"
        log "📍 CLI Binary: $(pwd)/welle-cli"

        # Verify binaries exist
        if [ -f "src/welle-io/welle-io" ]; then
            log "✅ GUI binary confirmed"
            log "🚀 To run Thailand DAB+ GUI:"
            log "   cd $(pwd)/src/welle-io && ./welle-io"
        else
            log "⚠️  GUI binary not found - check build output"
        fi

        if [ -f "welle-cli" ]; then
            log "✅ CLI binary confirmed"
            log "🌐 To run CLI web interface:"
            log "   cd $(pwd) && ./welle-cli -c 12B -w 8080"
        fi

        log ""
        log "🇹🇭 Thailand DAB+ Features Available:"
        log "   • Thai Character Set (0x0E) support"
        log "   • NBTC compliance checking"
        log "   • Bangkok frequencies (225.648 MHz, 227.360 MHz, 229.072 MHz)"
        log "   • Thai service information parsing"
        log "   • Regional coverage support"

    else
        error "Compilation failed! Run with --verbose for details"
    fi
}

build_docker() {
    log "🐳 Building Thailand DAB+ GUI with Docker (Experimental)"
    log "======================================================="
    log ""
    log "⚠️  Note: This approach is experimental and may take 10+ minutes"
    log "Recommended: Use './${SCRIPT_NAME} build' for faster local build"
    log ""

    if [ "$FORCE" != "1" ]; then
        read -p "Continue with Docker build? [y/N]: " -n 1 -r
        echo ""
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            log "Docker build cancelled."
            return 0
        fi
    fi

    # Docker build implementation would go here
    error "Docker build not yet implemented. Use './${SCRIPT_NAME} build' instead"
}

# Parse arguments
COMMAND=""
FORCE=0
VERBOSE=0
QUIET=0

while [[ $# -gt 0 ]]; do
    case $1 in
        install|check|build|docker|clean|help)
            COMMAND="$1"
            shift
            ;;
        --force)
            FORCE=1
            shift
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        --quiet)
            QUIET=1
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            error "Unknown option: $1. Use --help for usage information."
            ;;
    esac
done

# Default command if none provided
if [ -z "$COMMAND" ]; then
    show_help
    exit 0
fi

# Execute command
case $COMMAND in
    install)
        check_directory
        install_dependencies
        ;;
    check)
        check_directory
        check_dependencies
        ;;
    build)
        build_gui
        ;;
    docker)
        build_docker
        ;;
    clean)
        check_directory
        clean_build
        ;;
    help)
        show_help
        ;;
    *)
        error "Unknown command: $COMMAND"
        ;;
esac
