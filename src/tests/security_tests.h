/*
 *    Copyright (C) 2024
 *    welle.io Thailand DAB+ Security Tests Header
 *
 *    This file is part of the welle.io Thailand DAB+ security validation suite.
 *    
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 */

#ifndef SECURITY_TESTS_H
#define SECURITY_TESTS_H

/**
 * @brief Security test suite for welle.io Thailand DAB+ implementation
 * 
 * This test suite validates the security fixes for critical vulnerabilities:
 * - BUG-001: Buffer overflow in MOT parsing
 * - SECURITY-001: Thread-unsafe localtime()
 * - BUG-002: Raw pointer without ownership
 * 
 * Tests verify:
 * 1. Buffer overflow protection with bounds checking
 * 2. Thread safety of timestamp generation
 * 3. Const-correctness and memory safety
 * 4. General robustness against malformed inputs
 */
class SecurityTests {
public:
    SecurityTests();
    ~SecurityTests();
    
    /**
     * @brief Run all security tests
     * @return true if all tests pass, false otherwise
     */
    bool runAllTests();
    
    // ========================================================================
    // BUG-001: Buffer Overflow Protection Tests
    // ========================================================================
    
    /**
     * @brief Test MOT parsing with null pointer
     * Verifies: validateMOTHeader() null check prevents crash
     */
    bool testMOTBufferOverflowNullPointer();
    
    /**
     * @brief Test MOT parsing with zero length buffer
     * Verifies: Early validation prevents buffer access
     */
    bool testMOTBufferOverflowZeroLength();
    
    /**
     * @brief Test MOT parsing below minimum size (< 8 bytes)
     * Verifies: MOT_MIN_HEADER_SIZE enforcement
     */
    bool testMOTBufferOverflowMinimumSize();
    
    /**
     * @brief Test MOT header claiming length exceeds buffer
     * Verifies: Bounds checking prevents buffer overrun
     */
    bool testMOTBufferOverflowHeaderExceedsBuffer();
    
    /**
     * @brief Test integer overflow scenario (pos + 2 + header_length)
     * Verifies: Integer overflow prevention logic
     */
    bool testMOTBufferOverflowIntegerOverflow();
    
    /**
     * @brief Test maximum size enforcement (255 bytes)
     * Verifies: MAX_MOT_HEADER_SIZE limit
     */
    bool testMOTBufferOverflowMaximumSize();
    
    /**
     * @brief Test completely malformed MOT header
     * Verifies: Graceful handling of garbage data
     */
    bool testMOTBufferOverflowMalformedHeader();
    
    // ========================================================================
    // SECURITY-001: Thread Safety Tests
    // ========================================================================
    
    /**
     * @brief Test concurrent DLS parsing (10 threads)
     * Verifies: localtime_r/localtime_s thread safety
     */
    bool testThreadSafeDLSParsing();
    
    /**
     * @brief Test concurrent timestamp generation (100 threads)
     * Verifies: No timestamp corruption under high concurrency
     */
    bool testConcurrentTimestampGeneration();
    
    /**
     * @brief High concurrency stress test (1000 operations)
     * Verifies: Stability under realistic DAB+ receiver load
     */
    bool testHighConcurrencyStress();
    
    // ========================================================================
    // BUG-002: Pointer Safety Tests
    // ========================================================================
    
    /**
     * @brief Test const pointer correctness
     * Verifies: FIG1_Data accepts const uint8_t* without modification
     */
    bool testConstPointerCorrectness();
    
    /**
     * @brief Test FIG1_Data non-ownership semantics
     * Verifies: Data is copied, not referenced (safe to delete source)
     */
    bool testFIG1DataNonOwnership();
    
    // ========================================================================
    // General Memory Safety Tests
    // ========================================================================
    
    /**
     * @brief Test null pointer handling across all parsers
     * Verifies: All functions gracefully handle nullptr
     */
    bool testNullPointerHandling();
    
    /**
     * @brief Test empty buffer handling
     * Verifies: Zero-length buffers don't cause crashes
     */
    bool testEmptyBufferHandling();
    
    /**
     * @brief Test large input handling (1MB buffer)
     * Verifies: No excessive memory allocation or hangs
     */
    bool testLargeInputHandling();
    
    // ========================================================================
    // SecurityLogger Tests (Wave 2)
    // ========================================================================
    
    /**
     * @brief Test SecurityLogger basic functionality
     * Verifies: Logging works, counters increment
     */
    bool testSecurityLoggerBasic();
    
    /**
     * @brief Test SecurityLogger file logging
     * Verifies: Events written to file
     */
    bool testSecurityLoggerFileLogging();
    
    /**
     * @brief Test SecurityLogger thread safety
     * Verifies: Concurrent logging doesn't corrupt state
     */
    bool testSecurityLoggerThreadSafety();
    
    /**
     * @brief Test SecurityLogger severity filtering
     * Verifies: Minimum severity filter works
     */
    bool testSecurityLoggerSeverityFilter();
    
    /**
     * @brief Test SecurityLogger validation logging
     * Verifies: MOT validation failures are logged
     */
    bool testSecurityLoggerValidationIntegration();
    
    // ========================================================================
    // Wave 3: MOT Content Size Validation Tests
    // ========================================================================
    
    /**
     * @brief Test MOT content_size validation
     * Verifies: Excessive content_size is rejected
     */
    bool testMOTContentSizeValidation();
    
    /**
     * @brief Test MOT content_size boundary conditions
     * Verifies: Boundary values handled correctly
     */
    bool testMOTContentSizeBoundaries();
    
    /**
     * @brief Test MOT content_size security logging
     * Verifies: Excessive size triggers security log
     */
    bool testMOTContentSizeLogging();
    
    // ========================================================================
    // Wave 5: P1 Issue Fixes Tests
    // ========================================================================
    
    /**
     * @brief Test P1-002: UTF-8 validation in utf8ToUnicode()
     * Verifies: Null pointer check, continuation byte validation, bounds checking
     */
    bool testP1002_UTF8Validation();
    
    /**
     * @brief Test P1-003: Integer overflow prevention in UTF-8 parsing
     * Verifies: seq_length validation before arithmetic operations
     */
    bool testP1003_IntegerOverflowPrevention();
    
    /**
     * @brief Test P1-007: Integer overflow prevention in TIS-620 conversion
     * Verifies: Input length validation and safe reservation calculation
     */
    bool testP1007_TIS620IntegerOverflow();
    
    /**
     * @brief Test P1-005: SecurityLogger callback documentation
     * Verifies: Callback works correctly and documentation exists
     */
    bool testP1005_CallbackDocumentation();
    
    /**
     * @brief Test P1-009: File handling resource leak prevention
     * Verifies: Proper error checking and cleanup in file operations
     */
    bool testP1009_FileHandlingResourceLeak();
    
    /**
     * @brief Test P1-010: Performance optimization in numeral conversion
     * Verifies: Single-pass O(n) algorithm works correctly
     */
    bool testP1010_NumeralConversionPerformance();
};

#endif // SECURITY_TESTS_H
