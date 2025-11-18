#!/bin/bash
# E2E GUI Test Runner for welle.io with PDCA workflow verification
# This script builds and runs all e2e tests with zero console error checking

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test configuration
BUILD_DIR="${BUILD_DIR:-build}"
SOURCE_DIR="$(cd "$(dirname "$0")" && pwd)"
HEADLESS="${HEADLESS:-0}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}welle.io E2E GUI Test Suite${NC}"
echo -e "${BLUE}Multi-Agent PDCA Workflow Implementation${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Function to print section headers
print_section() {
    echo -e "\n${BLUE}>>> $1${NC}"
}

# Function to print success
print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

# Function to print error
print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Function to print warning
print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

# PLAN Phase
print_section "PLAN PHASE: Analyzing test infrastructure"
echo "Source directory: $SOURCE_DIR"
echo "Build directory: $BUILD_DIR"
echo "Headless mode: $([ $HEADLESS -eq 1 ] && echo 'YES' || echo 'NO')"

# Count test files
TEST_COUNT=$(find src/tests/e2e -name "test_*.cpp" | wc -l)
echo "Test files found: $TEST_COUNT"
print_success "Planning complete"

# DO Phase - Build
print_section "DO PHASE: Building test infrastructure"

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
if cmake .. -DBUILD_E2E_TESTS=ON -DBUILD_TESTING=ON; then
    print_success "CMake configuration successful"
else
    print_error "CMake configuration failed"
    exit 1
fi

# Build tests
echo "Building e2e tests..."
if make -j$(nproc) 2>&1 | tee build.log; then
    print_success "Build successful"
else
    print_error "Build failed - check build.log"
    exit 1
fi

# DO Phase - Run Tests
print_section "DO PHASE: Running e2e GUI tests"

# Set headless mode if requested
if [ $HEADLESS -eq 1 ]; then
    export QT_QPA_PLATFORM=offscreen
    print_warning "Running in headless mode (QT_QPA_PLATFORM=offscreen)"
fi

# Array of test executables
TESTS=(
    "test_general_view"
    "test_radio_view"
    "test_main_view"
    "test_info_page"
    "test_expert_view"
    "test_mot_view"
    "test_settings_pages"
    "test_components"
    "test_thai_components"
)

# Run each test
PASSED=0
FAILED=0
FAILED_TESTS=()

for test in "${TESTS[@]}"; do
    TEST_PATH="./src/tests/e2e/$test"

    if [ ! -f "$TEST_PATH" ]; then
        print_warning "Test executable not found: $test"
        continue
    fi

    echo ""
    echo "Running $test..."
    echo "----------------------------------------"

    if $TEST_PATH 2>&1 | tee "$test.log"; then
        print_success "$test PASSED"
        ((PASSED++))
    else
        print_error "$test FAILED"
        ((FAILED++))
        FAILED_TESTS+=("$test")
    fi
done

# CHECK Phase
print_section "CHECK PHASE: Verifying test results"

echo "Total tests run: $((PASSED + FAILED))"
echo "Passed: $PASSED"
echo "Failed: $FAILED"

# Check console errors in logs
echo ""
echo "Checking console logs for errors..."
CONSOLE_ERRORS=0

for test in "${TESTS[@]}"; do
    LOG_FILE="$test.log"
    if [ -f "$LOG_FILE" ]; then
        # Count console errors (QML warnings, critical errors)
        ERRORS=$(grep -c "QML.*Error\|Critical\|qrc:.*:[0-9]*:" "$LOG_FILE" || true)
        if [ $ERRORS -gt 0 ]; then
            print_warning "$test has $ERRORS console errors"
            ((CONSOLE_ERRORS += ERRORS))
        fi
    fi
done

if [ $CONSOLE_ERRORS -eq 0 ]; then
    print_success "ZERO console errors detected - Perfect!"
else
    print_warning "Total console errors found: $CONSOLE_ERRORS"
fi

# ACT Phase
print_section "ACT PHASE: Final assessment and recommendations"

if [ $FAILED -eq 0 ] && [ $CONSOLE_ERRORS -eq 0 ]; then
    print_success "ALL TESTS PASSED with ZERO CONSOLE ERRORS!"
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}✓ E2E Test Suite: PERFECT${NC}"
    echo -e "${GREEN}✓ PDCA Cycle: COMPLETE${NC}"
    echo -e "${GREEN}✓ Console Errors: ZERO${NC}"
    echo -e "${GREEN}========================================${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}Issues Found:${NC}"

    if [ $FAILED -gt 0 ]; then
        echo -e "${RED}✗ Failed tests: ${FAILED}${NC}"
        for test in "${FAILED_TESTS[@]}"; do
            echo -e "${RED}  - $test${NC}"
        done
    fi

    if [ $CONSOLE_ERRORS -gt 0 ]; then
        echo -e "${YELLOW}⚠ Console errors: ${CONSOLE_ERRORS}${NC}"
    fi

    echo -e "${RED}========================================${NC}"
    echo ""
    echo "Recommendations:"
    echo "1. Check individual test logs: $BUILD_DIR/*.log"
    echo "2. Fix failing tests and re-run"
    echo "3. Investigate console errors"
    echo "4. Re-run with: ./run-e2e-tests.sh"

    exit 1
fi
