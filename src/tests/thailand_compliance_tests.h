/*
 *    Copyright (C) 2024
 *    welle.io Thailand DAB+ Compliance Tests
 *
 *    This file is part of the welle.io Thailand DAB+ compliance implementation.
 *    
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 */

#ifndef __THAILAND_COMPLIANCE_TESTS_H
#define __THAILAND_COMPLIANCE_TESTS_H

/**
 * Thailand DAB+ Compliance Test Suite
 * 
 * Comprehensive testing framework for Thailand DAB+ compliance features
 * including character sets, frequency plans, service parsing, and NBTC compliance.
 */
class ThailandComplianceTests {
public:
    ThailandComplianceTests();
    ~ThailandComplianceTests();
    
    /**
     * Run all Thailand compliance tests
     * @return true if all tests pass
     */
    bool runAllTests();
    
    // Character Set Tests
    bool testThaiCharacterSet();
    bool testMixedLanguageText();
    bool testCharacterSetConversion();
    
    // Frequency Plan Tests
    bool testThailandFrequencies();
    bool testRegionalCoverage();
    
    // Service Information Tests
    bool testThaiServiceParsing();
    bool testDLSParsing();
    bool testMOTSlideShow();
    
    // NBTC Compliance Tests
    bool testNBTCCompliance();
    bool testEmergencyAlerts();
    
    // Text Conversion Tests
    bool testThaiTextConverter();
    bool testTextMetrics();
};

#endif // __THAILAND_COMPLIANCE_TESTS_H