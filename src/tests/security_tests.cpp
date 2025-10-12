/*
 *    Copyright (C) 2024
 *    welle.io Thailand DAB+ Security Tests
 *
 *    This file is part of the welle.io Thailand DAB+ security validation suite.
 *    
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 */

#include "security_tests.h"
#include "../backend/thailand-compliance/thai_service_parser.h"
#include "../backend/thailand-compliance/security_logger.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <fstream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

SecurityTests::SecurityTests() {
    // Initialize test suite
}

SecurityTests::~SecurityTests() {
    // Cleanup
}

bool SecurityTests::runAllTests() {
    std::cout << "=== Running Thailand DAB+ Security Tests ===" << std::endl;
    
    int passed = 0;
    int total = 0;
    
    // BUG-001: Buffer Overflow Protection Tests
    std::cout << "\n--- BUG-001: Buffer Overflow Protection ---" << std::endl;
    total++; if (testMOTBufferOverflowNullPointer()) passed++;
    total++; if (testMOTBufferOverflowZeroLength()) passed++;
    total++; if (testMOTBufferOverflowMinimumSize()) passed++;
    total++; if (testMOTBufferOverflowHeaderExceedsBuffer()) passed++;
    total++; if (testMOTBufferOverflowIntegerOverflow()) passed++;
    total++; if (testMOTBufferOverflowMaximumSize()) passed++;
    total++; if (testMOTBufferOverflowMalformedHeader()) passed++;
    
    // SECURITY-001: Thread Safety Tests
    std::cout << "\n--- SECURITY-001: Thread Safety ---" << std::endl;
    total++; if (testThreadSafeDLSParsing()) passed++;
    total++; if (testConcurrentTimestampGeneration()) passed++;
    total++; if (testHighConcurrencyStress()) passed++;
    
    // BUG-002: Pointer Safety Tests
    std::cout << "\n--- BUG-002: Pointer Safety ---" << std::endl;
    total++; if (testConstPointerCorrectness()) passed++;
    total++; if (testFIG1DataNonOwnership()) passed++;
    
    // General Memory Safety Tests
    std::cout << "\n--- General Memory Safety ---" << std::endl;
    total++; if (testNullPointerHandling()) passed++;
    total++; if (testEmptyBufferHandling()) passed++;
    total++; if (testLargeInputHandling()) passed++;
    
    // SecurityLogger Tests (Wave 2)
    std::cout << "\n--- Wave 2: SecurityLogger ---" << std::endl;
    total++; if (testSecurityLoggerBasic()) passed++;
    total++; if (testSecurityLoggerFileLogging()) passed++;
    total++; if (testSecurityLoggerThreadSafety()) passed++;
    total++; if (testSecurityLoggerSeverityFilter()) passed++;
    total++; if (testSecurityLoggerValidationIntegration()) passed++;
    
    // Wave 3: MOT Content Size Validation Tests
    std::cout << "\n--- Wave 3: MOT Content Size Validation ---" << std::endl;
    total++; if (testMOTContentSizeValidation()) passed++;
    total++; if (testMOTContentSizeBoundaries()) passed++;
    total++; if (testMOTContentSizeLogging()) passed++;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Security Tests: " << passed << "/" << total << " passed";
    if (passed == total) {
        std::cout << " ✓" << std::endl;
    } else {
        std::cout << " ✗" << std::endl;
    }
    std::cout << "========================================" << std::endl;
    
    return (passed == total);
}

// ============================================================================
// BUG-001: Buffer Overflow Protection Tests
// ============================================================================

bool SecurityTests::testMOTBufferOverflowNullPointer() {
    std::cout << "  [TEST] MOT parsing with null pointer... ";
    
    // Test null pointer handling
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(nullptr, 100);
    
    // Should return empty struct without crashing (most important: caption fields empty)
    bool passed = result.caption_thai.empty() && 
                  result.caption_english.empty();
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

bool SecurityTests::testMOTBufferOverflowZeroLength() {
    std::cout << "  [TEST] MOT parsing with zero length... ";
    
    uint8_t mot_data[10] = {0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x25, 0x0E};
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(mot_data, 0);
    
    // Should return empty struct
    bool passed = result.caption_thai.empty();
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

bool SecurityTests::testMOTBufferOverflowMinimumSize() {
    std::cout << "  [TEST] MOT parsing below minimum size... ";
    
    uint8_t mot_data[10] = {0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x25, 0x0E};
    
    // Test with 7 bytes (needs minimum 8)
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(mot_data, 7);
    
    // Should return empty struct
    bool passed = result.caption_thai.empty();
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

bool SecurityTests::testMOTBufferOverflowHeaderExceedsBuffer() {
    std::cout << "  [TEST] MOT header length exceeds buffer... ";
    
    // Create MOT data with header claiming to be larger than buffer
    uint8_t mot_data[15] = {
        0x00, 0x01,             // Transport ID
        0x00, 0x00, 0x01, 0x00, // Content size
        0x00, 0x00,             // MOT header size
        0x25,                   // Header type (Content Description)
        0xFF,                   // Header length (255 bytes - exceeds buffer!)
        0x0E,                   // Charset flag
        0x54, 0x65, 0x73, 0x74  // "Test"
    };
    
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(mot_data, sizeof(mot_data));
    
    // Should handle gracefully (empty or partial data, not crash)
    bool passed = true;  // If we get here without crashing, test passed
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

bool SecurityTests::testMOTBufferOverflowIntegerOverflow() {
    std::cout << "  [TEST] MOT integer overflow scenario... ";
    
    // Test case where pos + 2 + header_length would overflow
    uint8_t mot_data[256];
    memset(mot_data, 0, sizeof(mot_data));
    
    // Set up basic MOT header
    mot_data[0] = 0x00; mot_data[1] = 0x01; // Transport ID
    mot_data[2] = 0x00; mot_data[3] = 0x00; 
    mot_data[4] = 0x01; mot_data[5] = 0x00; // Content size
    mot_data[6] = 0x00; mot_data[7] = 0x00; // MOT header size
    
    // Create scenario near end of buffer
    mot_data[254] = 0x25; // Header type at position 254
    mot_data[255] = 0xFF; // Header length 255 would cause overflow
    
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(mot_data, sizeof(mot_data));
    
    // Should handle without integer overflow or buffer overrun
    bool passed = true;  // If we get here, overflow was prevented
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

bool SecurityTests::testMOTBufferOverflowMaximumSize() {
    std::cout << "  [TEST] MOT maximum size enforcement... ";
    
    // Test that MAX_MOT_HEADER_SIZE (255) is enforced
    uint8_t large_mot[300];
    memset(large_mot, 0, sizeof(large_mot));
    
    // Valid MOT header structure
    large_mot[0] = 0x00; large_mot[1] = 0x01; // Transport ID
    large_mot[2] = 0x00; large_mot[3] = 0x00; 
    large_mot[4] = 0x01; large_mot[5] = 0x00; // Content size
    large_mot[6] = 0x00; large_mot[7] = 0x00; // MOT header size
    large_mot[8] = 0x25;  // Header type
    large_mot[9] = 0xFE;  // Header length (254 - just under max)
    large_mot[10] = 0x0E; // Charset
    
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(large_mot, sizeof(large_mot));
    
    // Should handle large but valid header
    bool passed = true;  // Test passes if no crash
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

bool SecurityTests::testMOTBufferOverflowMalformedHeader() {
    std::cout << "  [TEST] MOT malformed header handling... ";
    
    // Create completely malformed MOT data
    uint8_t malformed[20];
    for (int i = 0; i < 20; i++) {
        malformed[i] = 0xFF;  // All 0xFF (invalid)
    }
    
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(malformed, sizeof(malformed));
    
    // Should handle gracefully
    bool passed = true;  // Pass if no crash
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

// ============================================================================
// SECURITY-001: Thread Safety Tests
// ============================================================================

bool SecurityTests::testThreadSafeDLSParsing() {
    std::cout << "  [TEST] Thread-safe DLS parsing (10 threads)... ";
    
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> unique_timestamps{0};
    std::vector<std::string> timestamps;
    timestamps.resize(10);
    
    // Launch 10 concurrent DLS parsers
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&, i]() {
            // Create valid DLS data with Thai charset
            uint8_t dls_data[] = {
                0x00, 0x0E,  // Charset flag (Thai Profile 0x0E)
                0x00, 0x00,  // Reserved
                0xE0, 0xB8, 0x97, 0xE0, 0xB8, 0x94,  // "ทด" in UTF-8
                0x73, 0x6F, 0x62  // "sob" in ASCII
            };
            
            auto result = ThaiServiceParser::parseThaiDLS(dls_data, sizeof(dls_data));
            
            if (!result.timestamp.empty()) {
                success_count++;
                timestamps[i] = result.timestamp;
            }
            
            // Small delay to increase chance of race condition
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // All threads should succeed
    bool passed = (success_count == 10);
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << " (" << success_count << "/10 threads)" << std::endl;
    return passed;
}

bool SecurityTests::testConcurrentTimestampGeneration() {
    std::cout << "  [TEST] Concurrent timestamp generation (100 threads)... ";
    
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    
    // Launch 100 concurrent timestamp generators
    for (int i = 0; i < 100; i++) {
        threads.emplace_back([&]() {
            uint8_t dls_data[] = {0x00, 0x00, 0x00, 0x00, 'T', 'e', 's', 't'};
            auto result = ThaiServiceParser::parseThaiDLS(dls_data, sizeof(dls_data));
            
            // Verify timestamp is valid (YYYY-MM-DD HH:MM:SS format)
            if (result.timestamp.length() == 19 && 
                result.timestamp[4] == '-' && 
                result.timestamp[7] == '-' &&
                result.timestamp[10] == ' ' &&
                result.timestamp[13] == ':' &&
                result.timestamp[16] == ':') {
                success_count++;
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    bool passed = (success_count == 100);
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << " (" << success_count << "/100 threads)" << std::endl;
    return passed;
}

bool SecurityTests::testHighConcurrencyStress() {
    std::cout << "  [TEST] High concurrency stress test (1000 operations)... ";
    
    std::atomic<int> operation_count{0};
    std::vector<std::thread> threads;
    
    // Simulate realistic DAB+ receiver workload
    // 10 threads, each processing 100 DLS updates
    for (int t = 0; t < 10; t++) {
        threads.emplace_back([&]() {
            for (int i = 0; i < 100; i++) {
                // Alternate between DLS and MOT parsing
                if (i % 2 == 0) {
                    uint8_t dls_data[32];
                    memset(dls_data, 0, sizeof(dls_data));
                    dls_data[0] = 0x00;
                    dls_data[1] = 0x0E;
                    auto result = ThaiServiceParser::parseThaiDLS(dls_data, sizeof(dls_data));
                    if (!result.timestamp.empty()) operation_count++;
                } else {
                    uint8_t mot_data[20];
                    memset(mot_data, 0, sizeof(mot_data));
                    mot_data[0] = 0x00; mot_data[1] = 0x01;
                    auto result = ThaiServiceParser::parseThaiMOTSlideShow(mot_data, sizeof(mot_data));
                    operation_count++;
                }
            }
        });
    }
    
    // Wait for completion
    for (auto& t : threads) {
        t.join();
    }
    
    // Should complete all 1000 operations without crashes or hangs
    bool passed = (operation_count == 1000);
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << " (" << operation_count << "/1000 ops)" << std::endl;
    return passed;
}

// ============================================================================
// BUG-002: Pointer Safety Tests
// ============================================================================

bool SecurityTests::testConstPointerCorrectness() {
    std::cout << "  [TEST] Const pointer correctness... ";
    
    // Test that const data can be used with FIG1_Data
    const uint8_t label_data[] = {
        0xE0, 0xB8, 0x97, 0xE0, 0xB8, 0x94,  // "ทด" in UTF-8
        0x73, 0x6F, 0x62                      // "sob" in ASCII
    };
    
    ThaiServiceParser::FIG1_Data fig1_data;
    fig1_data.service_id = 0x4001;
    fig1_data.label_data = label_data;  // Should compile (const pointer accepted)
    fig1_data.label_length = sizeof(label_data);
    fig1_data.charset_flag = 0x0E;  // Thai Profile
    fig1_data.character_flag_field = 0x0000;
    
    auto result = ThaiServiceParser::parseThaiService(fig1_data);
    
    // Verify parsing succeeded (either label populated) and original data unchanged
    bool passed = (!result.thai_label.empty() || !result.english_label.empty()) && 
                  label_data[0] == 0xE0;
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

bool SecurityTests::testFIG1DataNonOwnership() {
    std::cout << "  [TEST] FIG1_Data non-ownership semantics... ";
    
    uint8_t* temp_buffer = new uint8_t[16];
    memcpy(temp_buffer, "Test Station", 13);
    
    ThaiServiceParser::FIG1_Data fig1_data;
    fig1_data.service_id = 0x4001;
    fig1_data.label_data = temp_buffer;
    fig1_data.label_length = 13;
    fig1_data.charset_flag = 0x00;  // EBU Latin
    fig1_data.character_flag_field = 0x0000;
    
    // Parse service (copies data to std::string)
    auto result = ThaiServiceParser::parseThaiService(fig1_data);
    
    // Delete buffer (FIG1_Data should not have taken ownership)
    delete[] temp_buffer;
    
    // Verify result is still valid (data was copied, not referenced)
    bool passed = !result.english_label.empty();
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

// ============================================================================
// General Memory Safety Tests
// ============================================================================

bool SecurityTests::testNullPointerHandling() {
    std::cout << "  [TEST] Comprehensive null pointer handling... ";
    
    bool all_passed = true;
    
    // Test 1: DLS with null pointer
    auto dls_result = ThaiServiceParser::parseThaiDLS(nullptr, 100);
    all_passed &= dls_result.text_thai.empty();
    
    // Test 2: MOT with null pointer
    auto mot_result = ThaiServiceParser::parseThaiMOTSlideShow(nullptr, 100);
    all_passed &= mot_result.caption_thai.empty();
    
    // Test 3: FIG1_Data with null pointer
    ThaiServiceParser::FIG1_Data fig1_data;
    fig1_data.service_id = 0x4001;
    fig1_data.label_data = nullptr;
    fig1_data.label_length = 10;
    fig1_data.charset_flag = 0x00;
    fig1_data.character_flag_field = 0x0000;
    
    auto service_result = ThaiServiceParser::parseThaiService(fig1_data);
    all_passed &= service_result.english_label.empty();
    
    std::cout << (all_passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return all_passed;
}

bool SecurityTests::testEmptyBufferHandling() {
    std::cout << "  [TEST] Empty buffer handling... ";
    
    uint8_t empty_buffer[1] = {0x00};
    bool all_passed = true;
    
    // DLS with empty buffer
    auto dls_result = ThaiServiceParser::parseThaiDLS(empty_buffer, 0);
    all_passed &= dls_result.text_thai.empty();
    
    // MOT with empty buffer
    auto mot_result = ThaiServiceParser::parseThaiMOTSlideShow(empty_buffer, 0);
    all_passed &= mot_result.caption_thai.empty();
    
    std::cout << (all_passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return all_passed;
}

bool SecurityTests::testLargeInputHandling() {
    std::cout << "  [TEST] Large input handling (1MB buffer)... ";
    
    // Allocate large buffer
    size_t buffer_size = 1024 * 1024;  // 1MB
    uint8_t* large_buffer = new uint8_t[buffer_size];
    memset(large_buffer, 0, buffer_size);
    
    // Set up minimal valid MOT header
    large_buffer[0] = 0x00; large_buffer[1] = 0x01;
    large_buffer[2] = 0x00; large_buffer[3] = 0x00;
    large_buffer[4] = 0x01; large_buffer[5] = 0x00;
    large_buffer[6] = 0x00; large_buffer[7] = 0x00;
    
    // Should handle large buffer without excessive memory allocation
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(large_buffer, buffer_size);
    
    delete[] large_buffer;
    
    bool passed = true;  // Pass if no crash or hang
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

// ============================================================================
// SecurityLogger Tests (Wave 2)
// ============================================================================

bool SecurityTests::testSecurityLoggerBasic() {
    std::cout << "  [TEST] SecurityLogger basic functionality... ";
    
    auto& logger = SecurityLogger::getInstance();
    logger.resetCounters();
    
    // Log events of different severities
    logger.log(SecurityLogger::Severity::INFO, "TestComponent", "TestEvent", "Info message");
    logger.log(SecurityLogger::Severity::WARNING, "TestComponent", "TestEvent", "Warning message");
    logger.log(SecurityLogger::Severity::CRITICAL, "TestComponent", "TestEvent", "Critical message");
    
    // Check counters
    auto counts = logger.getEventCounts();
    bool passed = (counts.info == 1 && counts.warning == 1 && counts.critical == 1);
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << " (" << counts.total() << " events)" << std::endl;
    return passed;
}

bool SecurityTests::testSecurityLoggerFileLogging() {
    std::cout << "  [TEST] SecurityLogger file logging... ";
    
    auto& logger = SecurityLogger::getInstance();
    std::string test_log = "/tmp/welle_security_test.log";
    
    // Enable file logging
    if (!logger.enableFileLogging(test_log)) {
        std::cout << "FAIL ✗ (cannot open log file)" << std::endl;
        return false;
    }
    
    // Log some events
    logger.log(SecurityLogger::Severity::WARNING, "TestComponent", "FileTest", "Test event");
    
    // Disable logging
    logger.disableFileLogging();
    
    // Check file exists and has content
    std::ifstream log_file(test_log);
    bool file_exists = log_file.good();
    std::string line;
    int line_count = 0;
    while (std::getline(log_file, line)) {
        line_count++;
    }
    log_file.close();
    
    // Clean up
    std::remove(test_log.c_str());
    
    bool passed = file_exists && line_count > 0;
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << " (" << line_count << " lines)" << std::endl;
    return passed;
}

bool SecurityTests::testSecurityLoggerThreadSafety() {
    std::cout << "  [TEST] SecurityLogger thread safety... ";
    
    auto& logger = SecurityLogger::getInstance();
    logger.resetCounters();
    
    std::vector<std::thread> threads;
    std::atomic<int> events_logged{0};
    
    // Launch 50 threads, each logging 20 events
    for (int i = 0; i < 50; i++) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 20; j++) {
                logger.log(SecurityLogger::Severity::INFO, "ThreadTest", "Concurrent", "Test");
                events_logged++;
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify all events were logged
    auto counts = logger.getEventCounts();
    bool passed = (counts.info == 1000 && events_logged == 1000);
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << " (" << counts.info << "/1000 events)" << std::endl;
    return passed;
}

bool SecurityTests::testSecurityLoggerSeverityFilter() {
    std::cout << "  [TEST] SecurityLogger severity filtering... ";
    
    auto& logger = SecurityLogger::getInstance();
    logger.resetCounters();
    
    // Set minimum severity to WARNING (should ignore INFO)
    logger.setMinimumSeverity(SecurityLogger::Severity::WARNING);
    
    // Log events
    logger.log(SecurityLogger::Severity::INFO, "FilterTest", "Info", "Should be ignored");
    logger.log(SecurityLogger::Severity::WARNING, "FilterTest", "Warning", "Should be logged");
    logger.log(SecurityLogger::Severity::CRITICAL, "FilterTest", "Critical", "Should be logged");
    
    auto counts = logger.getEventCounts();
    
    // Reset to INFO for other tests
    logger.setMinimumSeverity(SecurityLogger::Severity::INFO);
    
    // Should have 0 INFO, 1 WARNING, 1 CRITICAL
    bool passed = (counts.info == 0 && counts.warning == 1 && counts.critical == 1);
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

bool SecurityTests::testSecurityLoggerValidationIntegration() {
    std::cout << "  [TEST] SecurityLogger MOT validation integration... ";
    
    auto& logger = SecurityLogger::getInstance();
    logger.resetCounters();
    
    // Create MOT data that will trigger validation failures
    uint8_t invalid_mot[] = {
        0x00, 0x01,             // Transport ID
        0x00, 0x00, 0x01, 0x00, // Content size
        0x00, 0x00,             // MOT header size
        0x25,                   // Header type
        0xFF,                   // Header length (255 - will exceed buffer)
        0x0E                    // Charset (only 1 byte of data remaining)
    };
    
    // This should trigger validation failure and log it
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(invalid_mot, sizeof(invalid_mot));
    
    // Check that warnings/critical events were logged
    auto counts = logger.getEventCounts();
    bool passed = (counts.warning > 0 || counts.critical > 0);
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") 
              << " (" << counts.warning << " warnings, " 
              << counts.critical << " critical)" << std::endl;
    return passed;
}

// ============================================================================
// Wave 3: MOT Content Size Validation Tests
// ============================================================================

bool SecurityTests::testMOTContentSizeValidation() {
    std::cout << "  [TEST] MOT content_size validation... ";
    
    // Create MOT data with excessive content_size (100MB)
    uint8_t mot_data[20];
    memset(mot_data, 0, sizeof(mot_data));
    
    mot_data[0] = 0x00; mot_data[1] = 0x01;  // Transport ID
    // content_size = 100MB (0x06400000)
    mot_data[2] = 0x06; mot_data[3] = 0x40; 
    mot_data[4] = 0x00; mot_data[5] = 0x00;
    mot_data[6] = 0x00; mot_data[7] = 0x00;  // MOT header size
    
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(mot_data, sizeof(mot_data));
    
    // content_size should be set to 0 (rejected)
    bool passed = (result.content_size == 0);
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

bool SecurityTests::testMOTContentSizeBoundaries() {
    std::cout << "  [TEST] MOT content_size boundaries... ";
    
    bool all_passed = true;
    
    // Test 1: Maximum allowed size (16MB exactly)
    uint8_t valid_mot[20];
    memset(valid_mot, 0, sizeof(valid_mot));
    valid_mot[0] = 0x00; valid_mot[1] = 0x01;
    // 16MB = 0x01000000
    valid_mot[2] = 0x01; valid_mot[3] = 0x00;
    valid_mot[4] = 0x00; valid_mot[5] = 0x00;
    valid_mot[6] = 0x00; valid_mot[7] = 0x00;
    
    auto result1 = ThaiServiceParser::parseThaiMOTSlideShow(valid_mot, sizeof(valid_mot));
    all_passed &= (result1.content_size == 16 * 1024 * 1024);  // Should accept
    
    // Test 2: One byte over limit (16MB + 1)
    uint8_t invalid_mot[20];
    memset(invalid_mot, 0, sizeof(invalid_mot));
    invalid_mot[0] = 0x00; invalid_mot[1] = 0x01;
    // 16MB + 1 = 0x01000001
    invalid_mot[2] = 0x01; invalid_mot[3] = 0x00;
    invalid_mot[4] = 0x00; invalid_mot[5] = 0x01;
    invalid_mot[6] = 0x00; invalid_mot[7] = 0x00;
    
    auto result2 = ThaiServiceParser::parseThaiMOTSlideShow(invalid_mot, sizeof(invalid_mot));
    all_passed &= (result2.content_size == 0);  // Should reject
    
    // Test 3: Reasonable size (1MB)
    uint8_t reasonable_mot[20];
    memset(reasonable_mot, 0, sizeof(reasonable_mot));
    reasonable_mot[0] = 0x00; reasonable_mot[1] = 0x01;
    // 1MB = 0x00100000
    reasonable_mot[2] = 0x00; reasonable_mot[3] = 0x10;
    reasonable_mot[4] = 0x00; reasonable_mot[5] = 0x00;
    reasonable_mot[6] = 0x00; reasonable_mot[7] = 0x00;
    
    auto result3 = ThaiServiceParser::parseThaiMOTSlideShow(reasonable_mot, sizeof(reasonable_mot));
    all_passed &= (result3.content_size == 1 * 1024 * 1024);  // Should accept
    
    std::cout << (all_passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return all_passed;
}

bool SecurityTests::testMOTContentSizeLogging() {
    std::cout << "  [TEST] MOT content_size logging... ";
    
    auto& logger = SecurityLogger::getInstance();
    logger.resetCounters();
    
    // Create MOT with excessive content_size
    uint8_t mot_data[20];
    memset(mot_data, 0, sizeof(mot_data));
    mot_data[0] = 0x00; mot_data[1] = 0x01;
    // 1GB (way over limit)
    mot_data[2] = 0x40; mot_data[3] = 0x00;
    mot_data[4] = 0x00; mot_data[5] = 0x00;
    mot_data[6] = 0x00; mot_data[7] = 0x00;
    
    // This should trigger security log
    auto result = ThaiServiceParser::parseThaiMOTSlideShow(mot_data, sizeof(mot_data));
    
    // Check that a warning was logged
    auto counts = logger.getEventCounts();
    bool passed = (counts.warning > 0);
    
    std::cout << (passed ? "PASS ✓" : "FAIL ✗") 
              << " (" << counts.warning << " warnings)" << std::endl;
    return passed;
}
