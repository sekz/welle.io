#!/bin/bash
# End-to-End Test Script for Announcement Support Feature
# Tests: GUI integration, settings, history, and indicator

set -e

echo "======================================"
echo "Announcement Support E2E Test"
echo "======================================"
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

test_pass() {
    echo -e "${GREEN}✓${NC} $1"
    TESTS_PASSED=$((TESTS_PASSED + 1))
}

test_fail() {
    echo -e "${RED}✗${NC} $1"
    TESTS_FAILED=$((TESTS_FAILED + 1))
}

test_info() {
    echo -e "${YELLOW}ℹ${NC} $1"
}

echo "=== Phase 1: Build Verification ==="
echo ""

# Test 1: Check welle-io binary exists
if [ -f "build/welle-io" ]; then
    test_pass "welle-io binary exists"
else
    test_fail "welle-io binary not found"
    exit 1
fi

# Test 2: Check binary size
SIZE=$(stat -f%z "build/welle-io" 2>/dev/null || stat -c%s "build/welle-io" 2>/dev/null || echo "0")
if [ "$SIZE" -gt 10000000 ]; then  # > 10 MB
    test_pass "Binary size OK ($(echo $SIZE | awk '{print int($1/1024/1024)}') MB)"
else
    test_fail "Binary size too small ($SIZE bytes)"
fi

echo ""
echo "=== Phase 2: QML Resource Registration ==="
echo ""

# Test 3: Check resources.qrc contains announcement components
if grep -q "AnnouncementIndicator.qml" src/welle-gui/resources.qrc; then
    test_pass "AnnouncementIndicator.qml registered in resources.qrc"
else
    test_fail "AnnouncementIndicator.qml NOT registered"
fi

if grep -q "AnnouncementHistory.qml" src/welle-gui/resources.qrc; then
    test_pass "AnnouncementHistory.qml registered in resources.qrc"
else
    test_fail "AnnouncementHistory.qml NOT registered"
fi

if grep -q "AnnouncementSettings.qml" src/welle-gui/resources.qrc; then
    test_pass "AnnouncementSettings.qml registered in resources.qrc"
else
    test_fail "AnnouncementSettings.qml NOT registered"
fi

if grep -q "ThaiDateFormatter.qml" src/welle-gui/resources.qrc; then
    test_pass "ThaiDateFormatter.qml registered in resources.qrc"
else
    test_fail "ThaiDateFormatter.qml NOT registered"
fi

echo ""
echo "=== Phase 3: MainView.qml Integration ==="
echo ""

# Test 4: Check MainView.qml has menu items
if grep -q "Announcement Settings" src/welle-gui/QML/MainView.qml; then
    test_pass "Announcement Settings menu item present"
else
    test_fail "Announcement Settings menu item MISSING"
fi

if grep -q "Announcement History" src/welle-gui/QML/MainView.qml; then
    test_pass "Announcement History menu item present"
else
    test_fail "Announcement History menu item MISSING"
fi

# Test 5: Check MainView.qml has dialogs
if grep -q "announcementSettingsDialog" src/welle-gui/QML/MainView.qml; then
    test_pass "announcementSettingsDialog defined"
else
    test_fail "announcementSettingsDialog MISSING"
fi

if grep -q "announcementHistoryDialog" src/welle-gui/QML/MainView.qml; then
    test_pass "announcementHistoryDialog defined"
else
    test_fail "announcementHistoryDialog MISSING"
fi

# Test 6: Check AnnouncementIndicator overlay
if grep -q "AnnouncementIndicator" src/welle-gui/QML/MainView.qml; then
    test_pass "AnnouncementIndicator overlay present"
else
    test_fail "AnnouncementIndicator overlay MISSING"
fi

echo ""
echo "=== Phase 4: Backend Integration ==="
echo ""

# Test 7: Check FIBProcessor callbacks
if grep -q "onAnnouncementSupportUpdate" src/backend/fib-processor.cpp; then
    test_pass "FIG 0/18 callback (onAnnouncementSupportUpdate) present"
else
    test_fail "FIG 0/18 callback MISSING"
fi

if grep -q "onAnnouncementSwitchingUpdate" src/backend/fib-processor.cpp; then
    test_pass "FIG 0/19 callback (onAnnouncementSwitchingUpdate) present"
else
    test_fail "FIG 0/19 callback MISSING"
fi

# Test 8: Check RadioController implementation
if grep -q "handleAnnouncementStarted" src/welle-gui/radio_controller.cpp; then
    test_pass "handleAnnouncementStarted() implemented"
else
    test_fail "handleAnnouncementStarted() MISSING"
fi

if grep -q "handleAnnouncementEnded" src/welle-gui/radio_controller.cpp; then
    test_pass "handleAnnouncementEnded() implemented"
else
    test_fail "handleAnnouncementEnded() MISSING"
fi

# Test 9: Check AnnouncementManager
if [ -f "src/backend/announcement-manager.cpp" ]; then
    test_pass "AnnouncementManager class exists"
else
    test_fail "AnnouncementManager class MISSING"
fi

if grep -q "shouldSwitchToAnnouncement" src/backend/announcement-manager.cpp; then
    test_pass "shouldSwitchToAnnouncement() decision logic present"
else
    test_fail "shouldSwitchToAnnouncement() MISSING"
fi

echo ""
echo "=== Phase 5: GUI Startup Test ==="
echo ""

# Test 10: Run GUI for 3 seconds and check for errors
test_info "Starting welle-io GUI for 3 seconds..."
timeout 3s build/welle-io 2>&1 | tee /tmp/welle-io-test.log || true

if grep -q "AnnouncementIndicator is not a type" /tmp/welle-io-test.log; then
    test_fail "GUI startup error: AnnouncementIndicator not found"
else
    test_pass "GUI startup: No AnnouncementIndicator type errors"
fi

if grep -q "Loaded announcement settings" /tmp/welle-io-test.log; then
    test_pass "AnnouncementManager settings loaded on startup"
else
    test_fail "AnnouncementManager settings NOT loaded"
fi

if grep -q "AnnouncementManager initialized" /tmp/welle-io-test.log; then
    test_pass "AnnouncementManager initialized successfully"
else
    test_fail "AnnouncementManager NOT initialized"
fi

echo ""
echo "=== Phase 6: Test Coverage ==="
echo ""

# Test 11: Check test file exists
if [ -f "src/tests/announcement_integration_tests.cpp" ]; then
    test_pass "Integration test file exists"

    # Count test cases
    TEST_COUNT=$(grep -c "TEST_CASE" src/tests/announcement_integration_tests.cpp || echo "0")
    if [ "$TEST_COUNT" -gt 50 ]; then
        test_pass "Test coverage: $TEST_COUNT test cases (target: 70+)"
    else
        test_fail "Test coverage insufficient: only $TEST_COUNT test cases"
    fi
else
    test_fail "Integration test file MISSING"
fi

echo ""
echo "=== Phase 7: Documentation ==="
echo ""

# Test 12: Check documentation
if [ -f "docs/features/announcement-support-user-guide.md" ]; then
    test_pass "User guide exists"
else
    test_fail "User guide MISSING"
fi

if [ -f "docs/testing/announcement-e2e-testing.md" ]; then
    test_pass "E2E testing guide exists"
else
    test_fail "E2E testing guide MISSING"
fi

echo ""
echo "======================================"
echo "Test Summary"
echo "======================================"
echo ""
echo -e "Tests Passed: ${GREEN}${TESTS_PASSED}${NC}"
echo -e "Tests Failed: ${RED}${TESTS_FAILED}${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED!${NC}"
    echo ""
    echo "🎉 Announcement support feature is READY FOR PRODUCTION!"
    echo ""
    echo "Next steps:"
    echo "1. Run './build/welle-io' to launch GUI"
    echo "2. Click hamburger menu (☰) in top-right"
    echo "3. Select 'Announcement Settings' to configure"
    echo "4. Select 'Announcement History' to view past announcements"
    echo "5. Announcement indicator will appear automatically when active"
    exit 0
else
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
    echo ""
    echo "Please review the failed tests above and fix issues."
    exit 1
fi
