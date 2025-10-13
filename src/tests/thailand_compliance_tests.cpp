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

#include "thailand_compliance_tests.h"
#include "../backend/charsets.h"
#include "../backend/thailand-compliance/nbtc_compliance_checker.h"
#include "../backend/thailand-compliance/thai_service_parser.h"
#include "../various/thai_text_converter.h"
#include <iostream>
#include <assert.h>
#include <string.h>

ThailandComplianceTests::ThailandComplianceTests() {
    // Initialize test cases
}

ThailandComplianceTests::~ThailandComplianceTests() {
    // Cleanup
}

bool ThailandComplianceTests::runAllTests() {
    std::cout << "=== Running Thailand DAB+ Compliance Tests ===" << std::endl;

    int passed = 0;
    int total = 0;

    // Character Set Tests
    total++; if (testThaiCharacterSet()) passed++;
    total++; if (testMixedLanguageText()) passed++;
    total++; if (testCharacterSetConversion()) passed++;

    // Frequency Plan Tests
    total++; if (testThailandFrequencies()) passed++;
    total++; if (testRegionalCoverage()) passed++;

    // Service Information Tests
    total++; if (testThaiServiceParsing()) passed++;
    total++; if (testDLSParsing()) passed++;
    total++; if (testMOTSlideShow()) passed++;

    // NBTC Compliance Tests
    total++; if (testNBTCCompliance()) passed++;
    total++; if (testEmergencyAlerts()) passed++;

    // Text Conversion Tests
    total++; if (testThaiTextConverter()) passed++;
    total++; if (testTextMetrics()) passed++;

    std::cout << "=== Test Results ===" << std::endl;
    std::cout << "Passed: " << passed << "/" << total << std::endl;
    std::cout << "Success Rate: " << (passed * 100 / total) << "%" << std::endl;

    return passed == total;
}

bool ThailandComplianceTests::testThaiCharacterSet() {
    std::cout << "Testing Thai Character Set (0x0E)..." << std::endl;

    // Test Thai character conversion
    const uint8_t thai_text[] = {0xE0, 0xB8, 0xAA, 0xE0, 0xB8, 0xA7, 0xE0, 0xB8, 0xB1, 0xE0, 0xB8, 0xAA, 0xE0, 0xB8, 0x94, 0xE0, 0xB8, 0xB5, 0x00}; // "สวัสดี" in UTF-8

    std::string result = toUtf8StringUsingCharset(reinterpret_cast<const char*>(thai_text), CharacterSet::ThaiProfile, sizeof(thai_text) - 1);

    if (result.empty()) {
        std::cout << "  FAIL: Thai character conversion returned empty string" << std::endl;
        return false;
    }

    std::cout << "  PASS: Thai character set conversion working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testMixedLanguageText() {
    std::cout << "Testing Mixed Thai-English Text..." << std::endl;

    // Test mixed language parsing
    std::string mixed_text = "Radio Thailand / วิทยุกระจายเสียงแห่งประเทศไทย";
    std::string thai_part, english_part;

    bool result = ThaiTextConverter::separateMixedLanguageText(mixed_text, thai_part, english_part);

    if (!result) {
        std::cout << "  FAIL: Mixed language text separation failed" << std::endl;
        return false;
    }

    if (thai_part.empty() && english_part.empty()) {
        std::cout << "  FAIL: No text parts extracted" << std::endl;
        return false;
    }

    std::cout << "  PASS: Mixed language text parsing working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testCharacterSetConversion() {
    std::cout << "Testing Character Set Conversion..." << std::endl;

    // Test ASCII text
    std::string ascii_text = "Radio Thailand";
    std::string result = toUtf8StringUsingCharset(ascii_text.c_str(), CharacterSet::EbuLatin, ascii_text.length());

    if (result != ascii_text) {
        std::cout << "  FAIL: ASCII conversion failed" << std::endl;
        return false;
    }

    std::cout << "  PASS: Character set conversion working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testThailandFrequencies() {
    std::cout << "Testing Thailand Frequency Plan..." << std::endl;

    // Test Bangkok primary frequencies
    auto freq_map = NBTCComplianceChecker::getThailandFrequencyMap();

    if (freq_map.find("12B") == freq_map.end() || freq_map["12B"] != 225648000) {
        std::cout << "  FAIL: Bangkok primary frequency 12B not found or incorrect" << std::endl;
        return false;
    }

    if (freq_map.find("12C") == freq_map.end() || freq_map["12C"] != 227360000) {
        std::cout << "  FAIL: Bangkok primary frequency 12C not found or incorrect" << std::endl;
        return false;
    }

    if (freq_map.find("12D") == freq_map.end() || freq_map["12D"] != 229072000) {
        std::cout << "  FAIL: Bangkok primary frequency 12D not found or incorrect" << std::endl;
        return false;
    }

    std::cout << "  PASS: Thailand frequency plan correct" << std::endl;
    return true;
}

bool ThailandComplianceTests::testRegionalCoverage() {
    std::cout << "Testing Regional Coverage..." << std::endl;

    // Test frequency allocation for Thailand
    bool result = NBTCComplianceChecker::isThailandFrequency(225648000); // 12B

    if (!result) {
        std::cout << "  FAIL: Thailand frequency not recognized" << std::endl;
        return false;
    }

    result = NBTCComplianceChecker::isThailandFrequency(300000000); // Invalid frequency

    if (result) {
        std::cout << "  FAIL: Invalid frequency incorrectly recognized as Thailand" << std::endl;
        return false;
    }

    std::cout << "  PASS: Regional coverage validation working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testThaiServiceParsing() {
    std::cout << "Testing Thai Service Parsing..." << std::endl;

    // Create mock FIG1 data
    std::string test_label = "วิทยุกระจายเสียงแห่งประเทศไทย";

    // P0-1 FIX: Create a proper mutable copy of the string data
    std::vector<uint8_t> label_buffer(test_label.begin(), test_label.end());
    ThaiServiceParser::FIG1_Data fig1_data{
        0x4001,                                      // service_id
        label_buffer.data(),                         // label_data (mutable buffer)
        static_cast<uint8_t>(test_label.length()),  // label_length
        0x0E,                                        // charset_flag (Thai Profile)
        0x0000                                       // character_flag_field
    };

    auto service_info = ThaiServiceParser::parseThaiService(fig1_data);

    if (service_info.service_id != 0x4001) {
        std::cout << "  FAIL: Service ID not parsed correctly" << std::endl;
        return false;
    }

    if (service_info.label_charset != CharacterSet::ThaiProfile) {
        std::cout << "  FAIL: Character set not parsed correctly" << std::endl;
        return false;
    }

    std::cout << "  PASS: Thai service parsing working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testDLSParsing() {
    std::cout << "Testing DLS Parsing..." << std::endl;

    // Create mock DLS data
    uint8_t dls_data[] = {0x10, 0x0E, 0x00, 0x00, 'T', 'e', 's', 't', ' ', 'D', 'L', 'S'};

    auto dls_info = ThaiServiceParser::parseThaiDLS(dls_data, sizeof(dls_data));

    if (dls_info.charset != CharacterSet::ThaiProfile) {
        std::cout << "  FAIL: DLS character set not parsed correctly" << std::endl;
        return false;
    }

    std::cout << "  PASS: DLS parsing working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testMOTSlideShow() {
    std::cout << "Testing MOT SlideShow..." << std::endl;

    // Create mock MOT data
    uint8_t mot_data[] = {0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x25, 0x05, 0x0E, 'T', 'e', 's', 't'};

    auto slideshow_info = ThaiServiceParser::parseThaiMOTSlideShow(mot_data, sizeof(mot_data));

    if (slideshow_info.transport_id != 1) {
        std::cout << "  FAIL: MOT transport ID not parsed correctly" << std::endl;
        return false;
    }

    std::cout << "  PASS: MOT SlideShow parsing working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testNBTCCompliance() {
    std::cout << "Testing NBTC Compliance..." << std::endl;

    // Create mock ensemble data
    NBTCComplianceChecker::DABEnsemble ensemble;
    ensemble.ensemble_id = 0x4001;
    ensemble.frequency = 225648000; // 12B
    ensemble.channel = "12B";
    ensemble.snr = 15.0;
    ensemble.signal_strength = 75;
    ensemble.fic_errors = 5;

    // Add mock service
    NBTCComplianceChecker::ServiceInfo service;
    service.service_id = 0x4001;
    service.thai_label = "วิทยุทดสอบ";
    service.english_label = "Test Radio";
    service.character_set = CharacterSet::ThaiProfile;
    service.emergency_support = true;

    ensemble.services.push_back(service);

    auto compliance_status = NBTCComplianceChecker::checkCompliance(ensemble);

    if (!compliance_status.frequency_compliant) {
        std::cout << "  FAIL: Frequency compliance check failed" << std::endl;
        return false;
    }

    if (!compliance_status.character_set_compliant) {
        std::cout << "  FAIL: Character set compliance check failed" << std::endl;
        return false;
    }

    if (compliance_status.compliance_score < 80) {
        std::cout << "  FAIL: Overall compliance score too low: " << compliance_status.compliance_score << std::endl;
        return false;
    }

    std::cout << "  PASS: NBTC compliance validation working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testEmergencyAlerts() {
    std::cout << "Testing Emergency Alerts..." << std::endl;

    // Create ensemble with emergency service
    NBTCComplianceChecker::DABEnsemble ensemble;

    NBTCComplianceChecker::ServiceInfo service;
    service.emergency_support = true;
    ensemble.services.push_back(service);

    bool result = NBTCComplianceChecker::checkEmergencyAlertCompliance(ensemble);

    if (!result) {
        std::cout << "  FAIL: Emergency alert compliance check failed" << std::endl;
        return false;
    }

    std::cout << "  PASS: Emergency alert validation working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testThaiTextConverter() {
    std::cout << "Testing Thai Text Converter..." << std::endl;

    // Test text metrics
    std::string thai_text = "สวัสดีครับ"; // Thai greeting
    auto metrics = ThaiTextConverter::analyzeThaiText(thai_text);

    if (metrics.character_count == 0) {
        std::cout << "  FAIL: Text metrics calculation failed" << std::endl;
        return false;
    }

    // Test Thai character detection
    bool contains_thai = ThaiTextConverter::containsThaiCharacters(thai_text);
    if (!contains_thai) {
        std::cout << "  FAIL: Thai character detection failed" << std::endl;
        return false;
    }

    std::cout << "  PASS: Thai text converter working" << std::endl;
    return true;
}

bool ThailandComplianceTests::testTextMetrics() {
    std::cout << "Testing Text Metrics..." << std::endl;

    std::string mixed_text = "Hello สวัสดี 123";
    auto metrics = ThaiTextConverter::analyzeThaiText(mixed_text);

    if (metrics.character_count == 0) {
        std::cout << "  FAIL: Text metrics character count failed" << std::endl;
        return false;
    }

    if (metrics.thai_character_count == 0) {
        std::cout << "  FAIL: Thai character count failed" << std::endl;
        return false;
    }

    if (metrics.english_character_count == 0) {
        std::cout << "  FAIL: English character count failed" << std::endl;
        return false;
    }

    std::cout << "  PASS: Text metrics calculation working" << std::endl;
    return true;
}
