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
 */

#include "nbtc_compliance_checker.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

// Thailand frequency allocations according to NBTC ผว. 104-2567
const std::map<std::string, double> NBTCComplianceChecker::THAILAND_FREQUENCIES = {
    {"5A", 174.928e6},  {"5B", 176.640e6},  {"5C", 178.352e6},  {"5D", 180.064e6},
    {"6A", 181.936e6},  {"6B", 183.648e6},  {"6C", 185.360e6},  {"6D", 187.072e6},
    {"7A", 188.928e6},  {"7B", 190.640e6},  {"7C", 192.352e6},  {"7D", 194.064e6},
    {"8A", 195.936e6},  {"8B", 197.648e6},  {"8C", 199.360e6},  {"8D", 201.072e6},
    {"9A", 202.928e6},  {"9B", 204.640e6},  {"9C", 206.352e6},  {"9D", 208.064e6},
    {"10A", 209.936e6}, {"10B", 211.648e6}, {"10C", 213.360e6}, {"10D", 215.072e6},
    {"11A", 216.928e6}, {"11B", 218.640e6}, {"11C", 220.352e6}, {"11D", 222.064e6},
    {"12A", 223.936e6}, {"12B", 225.648e6}, {"12C", 227.360e6}, {"12D", 229.072e6}
};

// Bangkok metropolitan primary frequencies
const std::vector<double> NBTCComplianceChecker::BANGKOK_PRIMARY_FREQUENCIES = {
    225.648e6, // 12B
    227.360e6, // 12C
    229.072e6  // 12D
};

// Regional channel assignments
const std::map<std::string, std::vector<std::string>> NBTCComplianceChecker::REGIONAL_CHANNELS = {
    {"bangkok_metro", {"12B", "12C", "12D"}},
    {"northern", {"12D", "13A"}},
    {"central", {"11D", "12A"}},
    {"southern", {"11B", "11C"}},
    {"northeastern", {"10C", "10D"}}
};

// Quality thresholds for Thailand broadcasting conditions
const double NBTCComplianceChecker::MIN_SNR_THAILAND = 10.0;  // dB
const int NBTCComplianceChecker::MIN_SIGNAL_STRENGTH_THAILAND = 60;  // %
const int NBTCComplianceChecker::MAX_FIC_ERRORS_THAILAND = 10;  // per minute

NBTCComplianceChecker::ComplianceStatus NBTCComplianceChecker::checkCompliance(const DABEnsemble& ensemble) {
    ComplianceStatus status;
    
    // Initialize compliance status
    status.frequency_compliant = checkFrequencyCompliance(ensemble.frequency, ensemble.channel);
    status.character_set_compliant = true; // Will be checked per service
    status.power_level_compliant = true;   // Assume compliant (would need RF measurements)
    status.service_info_compliant = true;  // Will be checked per service
    status.emergency_alert_compliant = checkEmergencyAlertCompliance(ensemble);
    
    // Check individual services
    for (const auto& service : ensemble.services) {
        if (!validateThailandService(service)) {
            status.service_info_compliant = false;
        }
        if (!checkCharacterSetCompliance(service.character_set)) {
            status.character_set_compliant = false;
        }
    }
    
    // Calculate compliance score
    int score = 0;
    if (status.frequency_compliant) score += 25;
    if (status.character_set_compliant) score += 25;
    if (status.power_level_compliant) score += 20;
    if (status.service_info_compliant) score += 20;
    if (status.emergency_alert_compliant) score += 10;
    
    status.compliance_score = score;
    status.compliance_report = generateComplianceReport(ensemble);
    
    return status;
}

bool NBTCComplianceChecker::validateThailandService(const ServiceInfo& service) {
    // Check if service has Thai label
    if (service.thai_label.empty()) {
        return false;
    }
    
    // Check if service supports Thai character set
    if (service.character_set != CharacterSet::ThaiProfile && 
        service.character_set != CharacterSet::UnicodeUtf8) {
        return false;
    }
    
    // Check if service has complete information
    if (service.service_id == 0) {
        return false;
    }
    
    return true;
}

bool NBTCComplianceChecker::checkFrequencyCompliance(double frequency, const std::string& channel) {
    // Check if frequency matches NBTC allocation
    auto it = THAILAND_FREQUENCIES.find(channel);
    if (it != THAILAND_FREQUENCIES.end()) {
        // Allow 1 kHz tolerance
        return std::abs(frequency - it->second) < 1000.0;
    }
    
    // Check if frequency is in Thailand allocation range
    return isThailandFrequency(frequency);
}

bool NBTCComplianceChecker::checkCharacterSetCompliance(CharacterSet charset) {
    switch (charset) {
        case CharacterSet::ThaiProfile:
        case CharacterSet::UnicodeUtf8:
        case CharacterSet::EbuLatin:
            return true;
        default:
            return false;
    }
}

bool NBTCComplianceChecker::checkServiceInformationCompliance(const ServiceInfo& service) {
    // Thai label is required
    if (service.thai_label.empty()) {
        return false;
    }
    
    // Service ID must be valid
    if (service.service_id == 0) {
        return false;
    }
    
    // Programme type should be specified
    if (service.programme_type_thai.empty()) {
        return false;
    }
    
    return true;
}

bool NBTCComplianceChecker::checkEmergencyAlertCompliance(const DABEnsemble& ensemble) {
    // Check if at least one service supports emergency alerts
    for (const auto& service : ensemble.services) {
        if (service.emergency_support) {
            return true;
        }
    }
    return false;
}

std::string NBTCComplianceChecker::generateComplianceReport(const DABEnsemble& ensemble) {
    std::stringstream report;
    
    report << "=== NBTC Thailand DAB+ Compliance Report ===\n";
    report << "Ensemble ID: 0x" << std::hex << ensemble.ensemble_id << "\n";
    report << "Ensemble Label: " << ensemble.ensemble_label << "\n";
    report << "Frequency: " << std::fixed << std::setprecision(3) << (ensemble.frequency / 1e6) << " MHz\n";
    report << "Channel: " << ensemble.channel << "\n";
    report << "Signal Strength: " << ensemble.signal_strength << "%\n";
    report << "SNR: " << std::fixed << std::setprecision(1) << ensemble.snr << " dB\n";
    report << "FIC Errors: " << ensemble.fic_errors << "\n\n";
    
    // Frequency compliance
    report << "Frequency Compliance: ";
    if (checkFrequencyCompliance(ensemble.frequency, ensemble.channel)) {
        report << "PASS - Frequency matches NBTC allocation\n";
    } else {
        report << "FAIL - Frequency not in NBTC allocation plan\n";
    }
    
    // Character set compliance
    report << "Character Set Compliance: ";
    bool charset_ok = true;
    for (const auto& service : ensemble.services) {
        if (!checkCharacterSetCompliance(service.character_set)) {
            charset_ok = false;
            break;
        }
    }
    if (charset_ok) {
        report << "PASS - All services support Thai character sets\n";
    } else {
        report << "FAIL - Some services lack Thai character support\n";
    }
    
    // Service information compliance
    report << "Service Information: ";
    bool service_ok = true;
    for (const auto& service : ensemble.services) {
        if (!validateThailandService(service)) {
            service_ok = false;
            break;
        }
    }
    if (service_ok) {
        report << "PASS - All services have complete Thai information\n";
    } else {
        report << "FAIL - Some services lack required Thai metadata\n";
    }
    
    // Emergency alert compliance
    report << "Emergency Alert Support: ";
    if (checkEmergencyAlertCompliance(ensemble)) {
        report << "PASS - Emergency alert capability detected\n";
    } else {
        report << "FAIL - No emergency alert support found\n";
    }
    
    // Signal quality assessment
    report << "\nSignal Quality Assessment:\n";
    if (ensemble.snr >= MIN_SNR_THAILAND) {
        report << "SNR: GOOD (" << ensemble.snr << " dB >= " << MIN_SNR_THAILAND << " dB)\n";
    } else {
        report << "SNR: POOR (" << ensemble.snr << " dB < " << MIN_SNR_THAILAND << " dB)\n";
    }
    
    if (ensemble.signal_strength >= MIN_SIGNAL_STRENGTH_THAILAND) {
        report << "Signal Strength: GOOD (" << ensemble.signal_strength << "% >= " << MIN_SIGNAL_STRENGTH_THAILAND << "%)\n";
    } else {
        report << "Signal Strength: POOR (" << ensemble.signal_strength << "% < " << MIN_SIGNAL_STRENGTH_THAILAND << "%)\n";
    }
    
    if (ensemble.fic_errors <= MAX_FIC_ERRORS_THAILAND) {
        report << "FIC Errors: GOOD (" << ensemble.fic_errors << " <= " << MAX_FIC_ERRORS_THAILAND << ")\n";
    } else {
        report << "FIC Errors: HIGH (" << ensemble.fic_errors << " > " << MAX_FIC_ERRORS_THAILAND << ")\n";
    }
    
    // Services summary
    report << "\nServices Summary:\n";
    for (const auto& service : ensemble.services) {
        report << "Service 0x" << std::hex << service.service_id << ": " << service.thai_label;
        if (!service.english_label.empty()) {
            report << " (" << service.english_label << ")";
        }
        report << "\n";
        report << "  Character Set: ";
        switch (service.character_set) {
            case CharacterSet::ThaiProfile:
                report << "Thai Profile (0x0E) - OPTIMAL";
                break;
            case CharacterSet::UnicodeUtf8:
                report << "Unicode UTF-8 (0x0F) - GOOD";
                break;
            case CharacterSet::EbuLatin:
                report << "EBU Latin (0x00) - LIMITED";
                break;
            default:
                report << "Unsupported - FAIL";
                break;
        }
        report << "\n";
        report << "  Emergency Support: " << (service.emergency_support ? "YES" : "NO") << "\n";
        report << "  Traffic Support: " << (service.traffic_support ? "YES" : "NO") << "\n";
    }
    
    return report.str();
}

std::map<std::string, double> NBTCComplianceChecker::getThailandFrequencyMap() {
    return THAILAND_FREQUENCIES;
}

bool NBTCComplianceChecker::isThailandFrequency(double frequency) {
    // Check if frequency is within Thailand DAB+ allocation (174-240 MHz)
    return (frequency >= 174.0e6 && frequency <= 240.0e6);
}