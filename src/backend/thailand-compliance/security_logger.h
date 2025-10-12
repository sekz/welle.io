/*
 *    Copyright (C) 2024
 *    welle.io Thailand DAB+ Security Logger
 *
 *    Thread-safe security event logging for DAB+ receiver
 *    
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 */

#ifndef SECURITY_LOGGER_H
#define SECURITY_LOGGER_H

#include <string>
#include <mutex>
#include <fstream>
#include <chrono>
#include <functional>

/**
 * @brief Thread-safe security event logger for Thailand DAB+ receiver
 * 
 * Logs security-related events such as:
 * - MOT validation failures
 * - Malformed broadcast data
 * - Buffer overflow attempts
 * - Suspicious input patterns
 * 
 * Design:
 * - Thread-safe using std::mutex
 * - Three severity levels: INFO, WARNING, CRITICAL
 * - Optional file logging
 * - Optional callback for external monitoring
 * - Minimal performance overhead (<1ms per log entry)
 */
class SecurityLogger {
public:
    /**
     * @brief Security event severity levels
     */
    enum class Severity {
        INFO,       ///< Informational: Normal security validation
        WARNING,    ///< Warning: Suspicious but handled safely
        CRITICAL    ///< Critical: Potential attack or serious malformation
    };

    /**
     * @brief Security event type
     */
    struct SecurityEvent {
        Severity severity;
        std::string component;      ///< Component name (e.g., "MOTParser", "DLSParser")
        std::string event_type;     ///< Event type (e.g., "BufferOverflow", "InvalidHeader")
        std::string description;    ///< Human-readable description
        std::string timestamp;      ///< ISO 8601 timestamp (YYYY-MM-DD HH:MM:SS)
        
        // Optional technical details
        size_t data_length;         ///< Length of data being processed
        size_t position;            ///< Position where issue occurred
        uint8_t header_value;       ///< Problematic header value (if applicable)
        
        SecurityEvent() 
            : severity(Severity::INFO)
            , data_length(0)
            , position(0)
            , header_value(0) {}
    };

    /**
     * @brief Callback function type for external monitoring
     * @param event The security event that occurred
     */
    using EventCallback = std::function<void(const SecurityEvent&)>;

    /**
     * @brief Get singleton instance
     * @return Reference to SecurityLogger singleton
     */
    static SecurityLogger& getInstance();

    /**
     * @brief Log a security event
     * @param event Security event to log
     */
    void log(const SecurityEvent& event);

    /**
     * @brief Log a security event (convenience overload)
     * @param severity Event severity
     * @param component Component name
     * @param event_type Event type
     * @param description Description
     */
    void log(Severity severity, 
             const std::string& component,
             const std::string& event_type,
             const std::string& description);

    /**
     * @brief Enable file logging
     * @param filepath Path to log file
     * @return true if file opened successfully
     */
    bool enableFileLogging(const std::string& filepath);

    /**
     * @brief Disable file logging
     */
    void disableFileLogging();

    /**
     * @brief Set event callback for external monitoring
     * @param callback Callback function
     */
    void setCallback(EventCallback callback);

    /**
     * @brief Clear event callback
     */
    void clearCallback();

    /**
     * @brief Set minimum severity level to log
     * @param min_severity Minimum severity (events below this are ignored)
     */
    void setMinimumSeverity(Severity min_severity);

    /**
     * @brief Get total number of events logged
     * @return Event count by severity
     */
    struct EventCounts {
        size_t info;
        size_t warning;
        size_t critical;
        size_t total() const { return info + warning + critical; }
    };
    EventCounts getEventCounts() const;

    /**
     * @brief Reset event counters
     */
    void resetCounters();

    /**
     * @brief Check if file logging is enabled
     * @return true if file logging active
     */
    bool isFileLoggingEnabled() const;

private:
    SecurityLogger();
    ~SecurityLogger();
    
    // Disable copy/move
    SecurityLogger(const SecurityLogger&) = delete;
    SecurityLogger& operator=(const SecurityLogger&) = delete;
    SecurityLogger(SecurityLogger&&) = delete;
    SecurityLogger& operator=(SecurityLogger&&) = delete;

    /**
     * @brief Generate ISO 8601 timestamp (thread-safe)
     * @return Timestamp string (YYYY-MM-DD HH:MM:SS)
     */
    std::string generateTimestamp() const;

    /**
     * @brief Convert severity to string
     * @param severity Severity level
     * @return String representation
     */
    static std::string severityToString(Severity severity);

    /**
     * @brief Format event as log entry
     * @param event Security event
     * @return Formatted log string
     */
    std::string formatEvent(const SecurityEvent& event) const;

    mutable std::mutex mutex_;        ///< Mutex for thread safety
    std::ofstream log_file_;          ///< Log file stream
    bool file_logging_enabled_;       ///< File logging enabled flag
    EventCallback callback_;          ///< External event callback
    Severity min_severity_;           ///< Minimum severity to log
    
    // Event counters
    mutable size_t info_count_;
    mutable size_t warning_count_;
    mutable size_t critical_count_;
};

// Convenience macros for common logging patterns
#define SECURITY_LOG_INFO(component, type, desc) \
    SecurityLogger::getInstance().log(SecurityLogger::Severity::INFO, component, type, desc)

#define SECURITY_LOG_WARNING(component, type, desc) \
    SecurityLogger::getInstance().log(SecurityLogger::Severity::WARNING, component, type, desc)

#define SECURITY_LOG_CRITICAL(component, type, desc) \
    SecurityLogger::getInstance().log(SecurityLogger::Severity::CRITICAL, component, type, desc)

#endif // SECURITY_LOGGER_H
