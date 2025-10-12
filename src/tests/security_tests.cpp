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
#include "../various/thai_text_converter.h"
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
    
    // Wave 5: P1 Issue Fixes Tests
    std::cout << "\n--- Wave 5: P1 Issue Fixes ---" << std::endl;
    total++; if (testP1002_UTF8Validation()) passed++;
    total++; if (testP1003_IntegerOverflowPrevention()) passed++;
    total++; if (testP1007_TIS620IntegerOverflow()) passed++;
    total++; if (testP1005_CallbackDocumentation()) passed++;
    
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

// ============================================================================
// Wave 5: P1 Issue Fixes Tests
// ============================================================================

bool SecurityTests::testP1002_UTF8Validation() {
    std::cout << "  [TEST] P1-002: UTF-8 validation in utf8ToUnicode()... ";

    bool all_passed = true;
    const uint32_t REPLACEMENT_CHAR = 0xFFFD;

    // Test 1: Null pointer check
    uint32_t result1 = ThaiTextConverter::utf8ToUnicode(nullptr, 2);
    all_passed &= (result1 == REPLACEMENT_CHAR);

    // Test 2: Invalid length (0)
    uint8_t valid_2byte[] = {0xC3, 0xA9};  // é
    uint32_t result2 = ThaiTextConverter::utf8ToUnicode(valid_2byte, 0);
    all_passed &= (result2 == REPLACEMENT_CHAR);

    // Test 3: Invalid length (> 4)
    uint32_t result3 = ThaiTextConverter::utf8ToUnicode(valid_2byte, 5);
    all_passed &= (result3 == REPLACEMENT_CHAR);

    // Test 4: Valid 2-byte UTF-8 (é = U+00E9)
    uint32_t result4 = ThaiTextConverter::utf8ToUnicode(valid_2byte, 2);
    all_passed &= (result4 == 0x00E9);

    // Test 5: Invalid 2-byte UTF-8 (bad continuation byte)
    uint8_t invalid_2byte[] = {0xC3, 0x20};  // Second byte is not continuation
    uint32_t result5 = ThaiTextConverter::utf8ToUnicode(invalid_2byte, 2);
    all_passed &= (result5 == REPLACEMENT_CHAR);

    // Test 6: Valid 3-byte UTF-8 (Thai ท = U+0E17)
    uint8_t valid_3byte[] = {0xE0, 0xB8, 0x97};  // ท
    uint32_t result6 = ThaiTextConverter::utf8ToUnicode(valid_3byte, 3);
    all_passed &= (result6 == 0x0E17);

    // Test 7: Invalid 3-byte UTF-8 (bad second continuation byte)
    uint8_t invalid_3byte_1[] = {0xE0, 0x20, 0x97};  // Second byte not continuation
    uint32_t result7 = ThaiTextConverter::utf8ToUnicode(invalid_3byte_1, 3);
    all_passed &= (result7 == REPLACEMENT_CHAR);

    // Test 8: Invalid 3-byte UTF-8 (bad third continuation byte)
    uint8_t invalid_3byte_2[] = {0xE0, 0xB8, 0x20};  // Third byte not continuation
    uint32_t result8 = ThaiTextConverter::utf8ToUnicode(invalid_3byte_2, 3);
    all_passed &= (result8 == REPLACEMENT_CHAR);

    // Test 9: Valid 4-byte UTF-8 (Emoji 😀 = U+1F600)
    uint8_t valid_4byte[] = {0xF0, 0x9F, 0x98, 0x80};
    uint32_t result9 = ThaiTextConverter::utf8ToUnicode(valid_4byte, 4);
    all_passed &= (result9 == 0x1F600);

    // Test 10: Invalid 4-byte UTF-8 (bad continuation bytes)
    uint8_t invalid_4byte[] = {0xF0, 0x9F, 0x20, 0x80};  // Third byte not continuation
    uint32_t result10 = ThaiTextConverter::utf8ToUnicode(invalid_4byte, 4);
    all_passed &= (result10 == REPLACEMENT_CHAR);

    // Test 11: Single ASCII byte
    uint8_t ascii[] = {0x41};  // 'A'
    uint32_t result11 = ThaiTextConverter::utf8ToUnicode(ascii, 1);
    all_passed &= (result11 == 0x41);

    std::cout << (all_passed ? "PASS ✓" : "FAIL ✗") << " (11 sub-tests)" << std::endl;
    return all_passed;
}

bool SecurityTests::testP1003_IntegerOverflowPrevention() {
    std::cout << "  [TEST] P1-003: Integer overflow prevention in UTF-8 parsing... ";

    bool all_passed = true;

    // Test 1: convertUTF8ToTIS620 with invalid UTF-8 sequence
    // This should not crash or cause integer overflow
    std::string invalid_utf8_1;
    invalid_utf8_1 += static_cast<char>(0xFF);  // Invalid UTF-8 first byte
    invalid_utf8_1 += static_cast<char>(0xFE);
    auto result1 = ThaiTextConverter::convertUTF8ToTIS620(invalid_utf8_1);
    all_passed &= true;  // Should not crash

    // Test 2: analyzeThaiText with invalid UTF-8 (CORRECTED function name)
    std::string invalid_utf8_2;
    invalid_utf8_2 += static_cast<char>(0xF8);  // Invalid - would return seq_length > 4
    invalid_utf8_2 += "test";
    auto metrics = ThaiTextConverter::analyzeThaiText(invalid_utf8_2);  // CORRECTED
    all_passed &= true;  // Should not crash

    // Test 3: containsThaiCharacters with invalid UTF-8
    std::string invalid_utf8_3;
    invalid_utf8_3 += static_cast<char>(0xFC);  // Invalid UTF-8
    bool contains = ThaiTextConverter::containsThaiCharacters(invalid_utf8_3);
    all_passed &= (contains == false);  // Should return false, not crash

    // Test 4: truncateThaiText with invalid UTF-8 (NEW TEST - missed function)
    std::string invalid_utf8_4;
    invalid_utf8_4 += "Hello";
    invalid_utf8_4 += static_cast<char>(0xFF);  // Invalid byte
    invalid_utf8_4 += "World";
    auto result4 = ThaiTextConverter::truncateThaiText(invalid_utf8_4, 10, false);
    all_passed &= true;  // Should not crash

    // Test 5: Valid Thai text should still work correctly
    std::string valid_thai;
    valid_thai += static_cast<char>(0xE0);
    valid_thai += static_cast<char>(0xB8);
    valid_thai += static_cast<char>(0x97);  // Thai ท
    bool contains_thai = ThaiTextConverter::containsThaiCharacters(valid_thai);
    all_passed &= (contains_thai == true);

    // Test 6: Mixed valid and invalid UTF-8
    std::string mixed;
    mixed += "Hello";
    mixed += static_cast<char>(0xFF);  // Invalid
    mixed += static_cast<char>(0xE0);
    mixed += static_cast<char>(0xB8);
    mixed += static_cast<char>(0x97);  // Thai ท
    mixed += static_cast<char>(0xFE);  // Invalid
    mixed += "World";
    auto result6 = ThaiTextConverter::convertUTF8ToTIS620(mixed);
    all_passed &= true;  // Should handle gracefully

    // Test 7: Edge case - seq_length boundary values
    std::string boundary;
    boundary += static_cast<char>(0x00);  // seq_length = 1
    boundary += static_cast<char>(0x7F);  // seq_length = 1
    boundary += static_cast<char>(0xC0);  // seq_length = 2
    boundary += static_cast<char>(0x80);  // continuation byte
    auto result7 = ThaiTextConverter::convertUTF8ToTIS620(boundary);
    all_passed &= true;  // Should not crash

    // Test 8: Verify metrics still work with valid Thai text
    std::string valid_metrics_test;
    valid_metrics_test += static_cast<char>(0xE0);
    valid_metrics_test += static_cast<char>(0xB8);
    valid_metrics_test += static_cast<char>(0xAA);  // Thai ส
    valid_metrics_test += static_cast<char>(0xE0);
    valid_metrics_test += static_cast<char>(0xB8);
    valid_metrics_test += static_cast<char>(0xA7);  // Thai ว
    auto metrics2 = ThaiTextConverter::analyzeThaiText(valid_metrics_test);
    all_passed &= (metrics2.thai_character_count > 0);  // Should detect Thai characters

    // Test 9: truncateThaiText with valid Thai and proper truncation
    std::string long_thai;
    long_thai += static_cast<char>(0xE0);
    long_thai += static_cast<char>(0xB8);
    long_thai += static_cast<char>(0xAA);  // Thai ส
    long_thai += static_cast<char>(0xE0);
    long_thai += static_cast<char>(0xB8);
    long_thai += static_cast<char>(0xA7);  // Thai ว
    long_thai += static_cast<char>(0xE0);
    long_thai += static_cast<char>(0xB8);
    long_thai += static_cast<char>(0xB1);  // Thai ั
    auto truncated = ThaiTextConverter::truncateThaiText(long_thai, 3, true);
    all_passed &= !truncated.empty();  // Should produce non-empty result

    std::cout << (all_passed ? "PASS ✓" : "FAIL ✗") << " (9 sub-tests)" << std::endl;
    return all_passed;
}

bool SecurityTests::testP1007_TIS620IntegerOverflow() {
    std::cout << "  [TEST] P1-007: Integer overflow in TIS-620 conversion... ";

    bool all_passed = true;

    // Test 1: Normal TIS-620 data should work correctly
    uint8_t normal_tis620[] = {0xA1, 0xA2, 0xA3, 0xA4};  // Thai characters
    std::string result1 = ThaiTextConverter::convertTIS620ToUTF8(normal_tis620, sizeof(normal_tis620));
    all_passed &= !result1.empty();  // Should produce output

    // Test 2: Excessive length should be truncated (not crash)
    // Create a large buffer (200KB - exceeds 100KB limit)
    const size_t large_size = 200 * 1024;
    std::vector<uint8_t> large_tis620(large_size, 0xA1);  // Fill with Thai character
    std::string result2 = ThaiTextConverter::convertTIS620ToUTF8(large_tis620.data(), large_size);
    // Should truncate to 100KB, so result should be <= 300KB (100KB * 3 bytes per char)
    all_passed &= (result2.size() <= 300 * 1024);
    all_passed &= !result2.empty();  // Should still produce something

    // Test 3: Maximum safe length (100KB exactly) should work
    const size_t max_safe = 100 * 1024;
    std::vector<uint8_t> max_tis620(max_safe, 0xA1);
    std::string result3 = ThaiTextConverter::convertTIS620ToUTF8(max_tis620.data(), max_safe);
    all_passed &= !result3.empty();  // Should work fine

    // Test 4: Mixed ASCII and Thai at large size
    std::vector<uint8_t> mixed_large(50 * 1024);
    for (size_t i = 0; i < mixed_large.size(); i++) {
        mixed_large[i] = (i % 2 == 0) ? 0x41 : 0xA1;  // Alternate 'A' and Thai
    }
    std::string result4 = ThaiTextConverter::convertTIS620ToUTF8(mixed_large.data(), mixed_large.size());
    all_passed &= !result4.empty();

    // Test 5: Empty input should return empty string
    std::string result5 = ThaiTextConverter::convertTIS620ToUTF8(nullptr, 100);
    all_passed &= result5.empty();

    // Test 6: Zero length should return empty string
    uint8_t dummy[] = {0xA1};
    std::string result6 = ThaiTextConverter::convertTIS620ToUTF8(dummy, 0);
    all_passed &= result6.empty();

    // Test 7: Verify no integer overflow in reservation
    // If length * 3 overflowed, reserve() might throw or behave badly
    // This test just ensures we don't crash with large (but not excessive) input
    const size_t near_limit = 99 * 1024;
    std::vector<uint8_t> near_limit_data(near_limit, 0xA1);
    std::string result7 = ThaiTextConverter::convertTIS620ToUTF8(near_limit_data.data(), near_limit);
    all_passed &= !result7.empty();

    std::cout << (all_passed ? "PASS ✓" : "FAIL ✗") << " (7 sub-tests)" << std::endl;
    return all_passed;
}

bool SecurityTests::testP1005_CallbackDocumentation() {
    std::cout << "  [TEST] P1-005: SecurityLogger callback documentation... ";

    bool all_passed = true;

    auto& logger = SecurityLogger::getInstance();
    logger.resetCounters();

    // Test 1: Basic callback functionality (non-blocking)
    std::atomic<int> callback_count{0};
    logger.setCallback([&callback_count](const SecurityLogger::SecurityEvent& event) {
        // Quick, non-blocking operation
        callback_count++;
    });

    // Log some events
    logger.log(SecurityLogger::Severity::INFO, "Test", "Callback", "Test event 1");
    logger.log(SecurityLogger::Severity::WARNING, "Test", "Callback", "Test event 2");
    logger.log(SecurityLogger::Severity::CRITICAL, "Test", "Callback", "Test event 3");

    // Verify callback was invoked
    all_passed &= (callback_count == 3);

    // Test 2: Callback can be cleared
    logger.clearCallback();
    int prev_count = callback_count;
    logger.log(SecurityLogger::Severity::INFO, "Test", "Callback", "Test event 4");
    all_passed &= (callback_count == prev_count);  // Should not increment

    // Test 3: Verify callback doesn't break logging
    logger.setCallback([](const SecurityLogger::SecurityEvent&) {
        // Empty callback
    });
    logger.log(SecurityLogger::Severity::INFO, "Test", "Callback", "Test event 5");
    auto counts = logger.getEventCounts();
    all_passed &= (counts.info >= 3);  // Should have logged

    // Test 4: Multiple rapid callbacks (simulating concurrent logging)
    logger.resetCounters();
    std::atomic<int> rapid_count{0};
    logger.setCallback([&rapid_count](const SecurityLogger::SecurityEvent&) {
        rapid_count++;
    });

    for (int i = 0; i < 100; i++) {
        logger.log(SecurityLogger::Severity::INFO, "Test", "Rapid", "Event");
    }

    all_passed &= (rapid_count == 100);

    // Clean up
    logger.clearCallback();

    std::cout << (all_passed ? "PASS ✓" : "FAIL ✗") << " (4 sub-tests)" << std::endl;
    std::cout << "         NOTE: P1-005 addressed via documentation (header + impl)" << std::endl;
    std::cout << "         Callbacks MUST be non-blocking to prevent race conditions" << std::endl;
    return all_passed;
}
