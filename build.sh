#!/bin/bash
# Build Guide for Content Fingerprinting Dashboard
# Quick reference for building and running the project

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "════════════════════════════════════════════════════════════════"
echo "  Content Fingerprinting Dashboard - Build Guide"
echo "════════════════════════════════════════════════════════════════"
echo ""

# ============ PREREQUISITES ============
echo "📋 Step 1: Check Prerequisites"
echo "─────────────────────────────────────────────────────────────────"

check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo "❌ $1 not found. Please install it first."
        return 1
    else
        echo "✅ $1 found: $(command -v $1)"
        return 0
    fi
}

check_command "g++" || exit 1
check_command "cmake" || exit 1
check_command "pkg-config" || exit 1

echo ""

# ============ SIMPLE BUILD (No GUI) ============
echo "🔨 Step 2: Build Options"
echo "─────────────────────────────────────────────────────────────────"
echo ""
echo "Option 1: Quick Build (CLI Only - compare tool)"
echo "  Command: bash $0 --quick"
echo "  Time: ~10 seconds"
echo ""
echo "Option 2: Full Build (CLI + GUI Dashboard)"
echo "  Command: bash $0 --full"
echo "  Time: ~30-60 seconds"
echo "  Requires: Qt6 development libraries"
echo ""
echo "Option 3: Help"
echo "  Command: bash $0 --help"
echo ""

# Parse arguments
if [[ "$1" == "--help" ]]; then
    echo "Usage: bash $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  --quick    Build CLI tool only (no Qt dependency)"
    echo "  --full     Build CLI + GUI dashboard (requires Qt6)"
    echo "  --install  Build and install to /usr/local"
    echo "  --test     Build and run tests"
    echo "  --clean    Remove build artifacts"
    echo "  --help     Show this help message"
    echo ""
    exit 0
fi

if [[ "$1" == "--clean" ]]; then
    echo "🧹 Cleaning build artifacts..."
    rm -rf "$BUILD_DIR"
    echo "✅ Clean complete"
    exit 0
fi

if [[ "$1" == "--quick" ]]; then
    echo "⚡ Quick Build (CLI Tool Only)"
    echo "─────────────────────────────────────────────────────────────────"
    
    cd "$PROJECT_DIR"
    
    echo "📦 Compiling compare tool..."
    g++ -std=c++17 -O3 -Wall -Wextra \
        -o "$PROJECT_DIR/compare" "$PROJECT_DIR/compare.cpp"
    
    echo "✅ Build complete!"
    echo ""
    echo "📍 Output: $PROJECT_DIR/compare"
    echo ""
    echo "🚀 Run examples:"
    echo "  $PROJECT_DIR/compare --help"
    echo "  echo 'test text' | $PROJECT_DIR/compare --threshold 0.01"
    echo ""
    exit 0
fi

if [[ "$1" == "--full" ]]; then
    echo "🚀 Full Build (CLI + GUI Dashboard)"
    echo "─────────────────────────────────────────────────────────────────"
    
    # Check for Qt6
    if ! pkg-config --exists Qt6Core Qt6Gui Qt6Widgets; then
        echo ""
        echo "❌ Qt6 development libraries not found!"
        echo ""
        echo "Install Qt6 on your system:"
        echo ""
        echo "  Ubuntu/Debian:"
        echo "    sudo apt-get install qt6-base-dev qt6-tools-dev"
        echo ""
        echo "  Fedora:"
        echo "    sudo dnf install qt6-qtbase-devel qt6-qttools-devel"
        echo ""
        echo "  macOS:"
        echo "    brew install qt@6"
        echo ""
        echo "  Or download from: https://www.qt.io/download"
        echo ""
        exit 1
    fi
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    echo "📋 Step 1: Configure with CMake..."
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_PREFIX_PATH="$(pkg-config --variable pc_path Qt6)/../../.." \
          "$PROJECT_DIR"
    
    echo ""
    echo "🔨 Step 2: Compile..."
    cmake --build . -j "$(nproc || echo 4)"
    
    echo ""
    echo "✅ Build complete!"
    echo ""
    echo "📍 Output binaries:"
    echo "  CLI:      $BUILD_DIR/compare"
    echo "  Dashboard: $BUILD_DIR/dashboard"
    echo ""
    echo "🚀 Run the applications:"
    echo "  $BUILD_DIR/compare --help"
    echo "  $BUILD_DIR/dashboard"
    echo ""
    exit 0
fi

if [[ "$1" == "--install" ]]; then
    echo "📦 Build and Install to System"
    echo "─────────────────────────────────────────────────────────────────"
    
    # First do full build
    bash "$0" --full
    
    echo ""
    echo "📍 Installing to /usr/local..."
    cd "$BUILD_DIR"
    sudo cmake --install . --prefix /usr/local
    
    echo ""
    echo "✅ Installation complete!"
    echo ""
    echo "🚀 System-wide commands:"
    echo "  compare"
    echo "  dashboard"
    echo ""
    exit 0
fi

if [[ "$1" == "--test" ]]; then
    echo "🧪 Build and Run Tests"
    echo "─────────────────────────────────────────────────────────────────"
    
    bash "$0" --full
    
    echo ""
    echo "📋 Running tests..."
    cd "$BUILD_DIR"
    ctest --output-on-failure
    
    exit 0
fi

# ============ DEFAULT INTERACTIVE BUILD ============
if [ -z "$1" ]; then
    echo ""
    echo "📦 Select build option:"
    echo ""
    echo "  1) Quick Build (CLI only, no Qt required)"
    echo "  2) Full Build (CLI + GUI, requires Qt6)"
    echo "  3) Install System-wide"
    echo "  4) Clean build artifacts"
    echo "  5) Exit"
    echo ""
    
    read -p "Enter choice [1-5]: " choice
    
    case $choice in
        1)
            bash "$0" --quick
            ;;
        2)
            bash "$0" --full
            ;;
        3)
            bash "$0" --install
            ;;
        4)
            bash "$0" --clean
            ;;
        5)
            echo "Exiting..."
            exit 0
            ;;
        *)
            echo "Invalid choice!"
            exit 1
            ;;
    esac
fi
