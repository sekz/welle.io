/*
 *    Copyright (C) 2024
 *    welle.io Thailand DAB+ Compliance
 *
 *    This file is part of the welle.io Thailand DAB+ compliance implementation.
 *    
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    welle.io is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 */

#ifndef __NBTC_COMPLIANCE_CHECKER_H
#define __NBTC_COMPLIANCE_CHECKER_H

#include <string>
#include <vector>
#include <map>
#include "charsets.h"

/**
 * NBTC Compliance Checker for Thailand DAB+ Broadcasting Standards
 * Implements compliance checking according to NBTC ผว. 104-2567
 * and ETSI EN 300 401 V2.1.1 with Thailand-specific requirements
 */
class NBTCComplianceChecker {
public:
    struct ComplianceStatus {
        bool frequency_compliant;      // NBTC frequency plan compliance
        bool character_set_compliant;  // Thai character support compliance
        bool power_level_compliant;    // NBTC power regulations compliance
        bool service_info_compliant;   // Service information standards compliance
        bool emergency_alert_compliant; // Emergency broadcasting compliance
        std::string compliance_report; // Detailed compliance report
        int compliance_score;          // Overall compliance score (0-100)
    };

    struct ServiceInfo {
        uint32_t service_id;
        std::string thai_label;
        std::string english_label;
        std::string programme_type_thai;
        std::string description_thai;
        CharacterSet character_set;
        bool emergency_support;
        bool traffic_support;
        double frequency;
        std::string channel;
    };

    struct DABEnsemble {
        uint16_t ensemble_id;
        std::string ensemble_label;
        double frequency;
        std::string channel;
        std::vector<ServiceInfo> services;
        int signal_strength;
        double snr;
        int fic_errors;
    };

    /**
     * Check overall compliance against NBTC ผว. 104-2567
     * @param ensemble DAB ensemble information
     * @return Compliance status and report
     */
    static ComplianceStatus checkCompliance(const DABEnsemble& ensemble);

    /**
     * Validate Thailand-specific service requirements
     * @param service Service information to validate
     * @return true if service meets Thailand requirements
     */
    static bool validateThailandService(const ServiceInfo& service);

    /**
     * Check frequency compliance with NBTC allocation plan
     * @param frequency Frequency in Hz
     * @param channel Channel identifier
     * @return true if frequency is compliant with NBTC plan
     */
    static bool checkFrequencyCompliance(double frequency, const std::string& channel);

    /**
     * Validate character set compliance for Thai broadcasting
     * @param charset Character set identifier
     * @return true if character set supports Thai text
     */
    static bool checkCharacterSetCompliance(CharacterSet charset);

    /**
     * Check service information completeness
     * @param service Service to validate
     * @return true if service information is complete
     */
    static bool checkServiceInformationCompliance(const ServiceInfo& service);

    /**
     * Validate emergency alert system support
     * @param ensemble Ensemble to check
     * @return true if emergency alert support is compliant
     */
    static bool checkEmergencyAlertCompliance(const DABEnsemble& ensemble);

    /**
     * Generate detailed compliance report
     * @param ensemble Ensemble to analyze
     * @return Formatted compliance report string
     */
    static std::string generateComplianceReport(const DABEnsemble& ensemble);

    /**
     * Get Thailand frequency allocation map
     * @return Map of channel to frequency mappings for Thailand
     */
    static std::map<std::string, double> getThailandFrequencyMap();

    /**
     * Check if frequency is in Thailand DAB+ allocation
     * @param frequency Frequency in Hz
     * @return true if frequency is allocated for Thailand DAB+
     */
    static bool isThailandFrequency(double frequency);

private:
    // Thailand-specific frequency allocations (NBTC)
    static const std::map<std::string, double> THAILAND_FREQUENCIES;
    
    // Bangkok metropolitan frequencies
    static const std::vector<double> BANGKOK_PRIMARY_FREQUENCIES;
    
    // Regional frequency assignments
    static const std::map<std::string, std::vector<std::string>> REGIONAL_CHANNELS;
    
    // Minimum signal quality requirements for Thailand
    static const double MIN_SNR_THAILAND;
    static const int MIN_SIGNAL_STRENGTH_THAILAND;
    static const int MAX_FIC_ERRORS_THAILAND;
};

#endif // __NBTC_COMPLIANCE_CHECKER_H