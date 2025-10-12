/*
 *    Copyright (C) 2024
 *    welle.io Thailand DAB+ Security Logger Implementation
 *
 *    Thread-safe security event logging for DAB+ receiver
 */

#include "security_logger.h"
#include <sstream>
#include <iomanip>
#include <ctime>

SecurityLogger::SecurityLogger()
    : file_logging_enabled_(false)
    , min_severity_(Severity::INFO)
    , info_count_(0)
    , warning_count_(0)
    , critical_count_(0)
{
}

SecurityLogger::~SecurityLogger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

SecurityLogger& SecurityLogger::getInstance() {
    static SecurityLogger instance;
    return instance;
}

void SecurityLogger::log(const SecurityEvent& event) {
    // Check minimum severity filter
    if (event.severity < min_severity_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // Update counters
    switch (event.severity) {
        case Severity::INFO:
            ++info_count_;
            break;
        case Severity::WARNING:
            ++warning_count_;
            break;
        case Severity::CRITICAL:
            ++critical_count_;
            break;
    }

    // Format log entry
    std::string log_entry = formatEvent(event);

    // Write to file if enabled
    if (file_logging_enabled_ && log_file_.is_open()) {
        log_file_ << log_entry << std::endl;
        log_file_.flush();  // Ensure immediate write for security events
    }

    // Call external callback if set
    if (callback_) {
        try {
            callback_(event);
        } catch (...) {
            // Swallow callback exceptions to prevent disrupting receiver
        }
    }
}

void SecurityLogger::log(Severity severity, 
                         const std::string& component,
                         const std::string& event_type,
                         const std::string& description)
{
    SecurityEvent event;
    event.severity = severity;
    event.component = component;
    event.event_type = event_type;
    event.description = description;
    event.timestamp = generateTimestamp();
    
    log(event);
}

bool SecurityLogger::enableFileLogging(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Close existing file if open
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    // Open new log file
    log_file_.open(filepath, std::ios::app);  // Append mode
    file_logging_enabled_ = log_file_.is_open();
    
    if (file_logging_enabled_) {
        // Write header
        log_file_ << "\n========================================\n";
        log_file_ << "Security Log Started: " << generateTimestamp() << "\n";
        log_file_ << "welle.io Thailand DAB+ Security Logger\n";
        log_file_ << "========================================\n" << std::endl;
    }
    
    return file_logging_enabled_;
}

void SecurityLogger::disableFileLogging() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (log_file_.is_open()) {
        log_file_ << "\n========================================\n";
        log_file_ << "Security Log Stopped: " << generateTimestamp() << "\n";
        log_file_ << "========================================\n" << std::endl;
        log_file_.close();
    }
    
    file_logging_enabled_ = false;
}

void SecurityLogger::setCallback(EventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = callback;
}

void SecurityLogger::clearCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = nullptr;
}

void SecurityLogger::setMinimumSeverity(Severity min_severity) {
    std::lock_guard<std::mutex> lock(mutex_);
    min_severity_ = min_severity;
}

SecurityLogger::EventCounts SecurityLogger::getEventCounts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    EventCounts counts;
    counts.info = info_count_;
    counts.warning = warning_count_;
    counts.critical = critical_count_;
    return counts;
}

void SecurityLogger::resetCounters() {
    std::lock_guard<std::mutex> lock(mutex_);
    info_count_ = 0;
    warning_count_ = 0;
    critical_count_ = 0;
}

bool SecurityLogger::isFileLoggingEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return file_logging_enabled_;
}

std::string SecurityLogger::generateTimestamp() const {
    // Thread-safe timestamp generation (same as DLS parser fix)
    std::time_t now = std::time(nullptr);
    std::tm local_time_buf{};

#ifdef _WIN32
    // Windows: localtime_s(dest, src)
    if (localtime_s(&local_time_buf, &now) != 0) {
        return "1970-01-01 00:00:00";  // Epoch fallback
    }
#else
    // POSIX (Linux/macOS/Android): localtime_r(src, dest)
    if (localtime_r(&now, &local_time_buf) == nullptr) {
        return "1970-01-01 00:00:00";  // Epoch fallback
    }
#endif

    std::stringstream ss;
    ss << std::put_time(&local_time_buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string SecurityLogger::severityToString(Severity severity) {
    switch (severity) {
        case Severity::INFO:     return "INFO";
        case Severity::WARNING:  return "WARNING";
        case Severity::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

std::string SecurityLogger::formatEvent(const SecurityEvent& event) const {
    std::stringstream ss;
    
    // Format: [TIMESTAMP] [SEVERITY] Component:EventType - Description
    ss << "[" << event.timestamp << "] ";
    ss << "[" << std::setw(8) << std::left << severityToString(event.severity) << "] ";
    ss << event.component << ":" << event.event_type;
    
    if (!event.description.empty()) {
        ss << " - " << event.description;
    }
    
    // Add technical details if present
    if (event.data_length > 0 || event.position > 0) {
        ss << " (";
        if (event.data_length > 0) {
            ss << "length=" << event.data_length;
        }
        if (event.position > 0) {
            if (event.data_length > 0) ss << ", ";
            ss << "pos=" << event.position;
        }
        if (event.header_value != 0) {
            if (event.data_length > 0 || event.position > 0) ss << ", ";
            ss << "header=0x" << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(event.header_value) << std::dec;
        }
        ss << ")";
    }
    
    return ss.str();
}
