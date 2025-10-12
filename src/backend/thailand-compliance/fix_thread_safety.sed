/\/\/ Generate timestamp/,/dls_info\.timestamp = ss\.str();/ {
  /\/\/ Generate timestamp/ {
    a\    // Generate timestamp (thread-safe)\
    std::time_t now = std::time(nullptr);\
    std::tm local_time_buf{};\
    \
#ifdef _WIN32\
    // Windows: localtime_s(dest, src)\
    if (localtime_s(&local_time_buf, &now) != 0) {\
        dls_info.timestamp = "1970-01-01 00:00:00";  // Epoch fallback\
        return dls_info;\
    }\
#else\
    // POSIX (Linux/macOS/Android): localtime_r(src, dest)\
    if (localtime_r(&now, &local_time_buf) == nullptr) {\
        dls_info.timestamp = "1970-01-01 00:00:00";  // Epoch fallback\
        return dls_info;\
    }\
#endif\
    \
    std::stringstream ss;\
    ss << std::put_time(&local_time_buf, "%Y-%m-%d %H:%M:%S");\
    dls_info.timestamp = ss.str();
    d
  }
  d
}
