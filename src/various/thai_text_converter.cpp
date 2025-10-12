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

#include "thai_text_converter.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

// Thai numeral mappings
const std::map<char, std::string> ThaiTextConverter::ARABIC_TO_THAI_NUMERALS = {
    {'0', "๐"}, {'1', "๑"}, {'2', "๒"}, {'3', "๓"}, {'4', "๔"},
    {'5', "๕"}, {'6', "๖"}, {'7', "๗"}, {'8', "๘"}, {'9', "๙"}
};

const std::map<std::string, char> ThaiTextConverter::THAI_TO_ARABIC_NUMERALS = {
    {"๐", '0'}, {"๑", '1'}, {"๒", '2'}, {"๓", '3'}, {"๔", '4'},
    {"๕", '5'}, {"๖", '6'}, {"๗", '7'}, {"๘", '8'}, {"๙", '9'}
};

// Thai tone marks (U+0E48 - U+0E4B)
const std::vector<uint16_t> ThaiTextConverter::THAI_TONE_MARKS = {
    0x0E48, // Mai Ek (่)
    0x0E49, // Mai Tho (้)
    0x0E4A, // Mai Tri (๊)
    0x0E4B  // Mai Chattawa (๋)
};

// Thai vowel signs
const std::vector<uint16_t> ThaiTextConverter::THAI_VOWEL_SIGNS = {
    0x0E31, // Mai Han-Akat (ั)
    0x0E34, // Sara I (ิ)
    0x0E35, // Sara Ii (ี)
    0x0E36, // Sara Ue (ึ)
    0x0E37, // Sara Uee (ื)
    0x0E38, // Sara U (ุ)
    0x0E39, // Sara Uu (ู)
    0x0E3A, // Phinthu (ฺ)
    0x0E47  // Maitaikhu (็)
};

// Common Thai words for basic segmentation
const std::vector<std::string> ThaiTextConverter::THAI_COMMON_WORDS = {
    "และ", "หรือ", "แต่", "เพราะ", "ดังนั้น", "อย่างไรก็ตาม",
    "ที่", "ใน", "บน", "ของ", "จาก", "ไป", "มา", "อยู่", "เป็น", "มี",
    "วิทยุ", "การออกอากาศ", "ข่าว", "เพลง", "รายการ", "สถานี"
};

std::string ThaiTextConverter::convertTIS620ToUTF8(const uint8_t* tis620_data, size_t length) {
    // P1-007 Fix: Prevent integer overflow in reservation calculation
    const size_t MAX_TIS620_INPUT = 100 * 1024;  // 100KB reasonable limit
    
    if (!tis620_data || length == 0) {
        return "";
    }
    
    // Validate input length to prevent excessive memory allocation
    if (length > MAX_TIS620_INPUT) {
        length = MAX_TIS620_INPUT;  // Truncate to safe limit
    }
    
    std::string result;
    // Reserve space for UTF-8 expansion (Thai chars = 3 bytes, ASCII = 1 byte)
    // Use length * 3 instead of length * 2 to be safe for all-Thai input
    result.reserve(length * 3);
    
    for (size_t i = 0; i < length; i++) {
        uint8_t byte = tis620_data[i];
        
        if (byte >= 0x00 && byte <= 0x7F) {
            // ASCII range - direct mapping
            result += static_cast<char>(byte);
        } else if (byte >= 0xA1 && byte <= 0xFE) {
            // Thai character range - convert to UTF-8
            uint16_t unicode = 0x0E00 + (byte - 0xA0);
            auto utf8_bytes = unicodeToUTF8(unicode);
            for (uint8_t utf8_byte : utf8_bytes) {
                result += static_cast<char>(utf8_byte);
            }
        } else {
            // Undefined or control character - replace with space
            result += ' ';
        }
    }
    
    return result;
}

std::vector<uint8_t> ThaiTextConverter::convertUTF8ToTIS620(const std::string& utf8_text) {
    std::vector<uint8_t> result;
    
    for (size_t i = 0; i < utf8_text.length(); ) {
        uint8_t first_byte = static_cast<uint8_t>(utf8_text[i]);
        int seq_length = getUTF8SequenceLength(first_byte);
        
        // P1-003 Fix: Validate seq_length before arithmetic to prevent integer overflow
        if (seq_length < 1 || seq_length > 4) {
            i++; // Skip invalid byte
            continue;
        }
        
        if (i + seq_length > utf8_text.length()) {
            break; // Invalid sequence
        }
        
        if (seq_length == 1) {
            // ASCII character
            result.push_back(first_byte);
            i++;
        } else {
            // Multi-byte UTF-8 sequence
            uint32_t codepoint = utf8ToUnicode(
                reinterpret_cast<const uint8_t*>(&utf8_text[i]), seq_length);
            
            if (codepoint >= 0x0E01 && codepoint <= 0x0E5E) {
                // Thai character - convert to TIS-620
                uint8_t tis620_byte = static_cast<uint8_t>(0xA0 + (codepoint - 0x0E00));
                result.push_back(tis620_byte);
            } else if (codepoint <= 0x7F) {
                // ASCII range
                result.push_back(static_cast<uint8_t>(codepoint));
            } else {
                // Non-Thai Unicode character - replace with '?'
                result.push_back('?');
            }
            
            i += seq_length;
        }
    }
    
    return result;
}

std::string ThaiTextConverter::normalizeThaiText(const std::string& thai_text) {
    if (thai_text.empty()) {
        return "";
    }
    
    std::string result;
    result.reserve(thai_text.length());
    
    // Basic normalization: remove extra whitespace, normalize line endings
    bool prev_was_space = false;
    
    for (size_t i = 0; i < thai_text.length(); i++) {
        char c = thai_text[i];
        
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!prev_was_space && !result.empty()) {
                result += ' ';
                prev_was_space = true;
            }
        } else {
            result += c;
            prev_was_space = false;
        }
    }
    
    // Remove trailing whitespace
    if (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }
    
    return result;
}

std::vector<std::string> ThaiTextConverter::segmentThaiWords(const std::string& thai_text) {
    std::vector<std::string> words;
    
    if (thai_text.empty()) {
        return words;
    }
    
    // Simple word segmentation based on spaces and common Thai words
    std::string current_word;
    
    for (size_t i = 0; i < thai_text.length(); i++) {
        char c = thai_text[i];
        
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!current_word.empty()) {
                words.push_back(current_word);
                current_word.clear();
            }
        } else {
            current_word += c;
        }
    }
    
    // Add the last word
    if (!current_word.empty()) {
        words.push_back(current_word);
    }
    
    return words;
}

std::string ThaiTextConverter::formatThaiTextForDisplay(const std::string& thai_text, int max_width) {
    if (thai_text.empty() || max_width <= 0) {
        return thai_text;
    }
    
    auto words = segmentThaiWords(thai_text);
    std::string result;
    int current_line_length = 0;
    
    for (const auto& word : words) {
        int word_length = static_cast<int>(word.length()); // Simplified character counting
        
        if (current_line_length + word_length + 1 > max_width && current_line_length > 0) {
            // Start new line
            result += '\n';
            current_line_length = 0;
        }
        
        if (current_line_length > 0) {
            result += ' ';
            current_line_length++;
        }
        
        result += word;
        current_line_length += word_length;
    }
    
    return result;
}

ThaiTextConverter::ThaiTextMetrics ThaiTextConverter::analyzeThaiText(const std::string& text) {
    ThaiTextMetrics metrics = {};
    
    if (text.empty()) {
        return metrics;
    }
    
    size_t i = 0;
    while (i < text.length()) {
        uint8_t first_byte = static_cast<uint8_t>(text[i]);
        int seq_length = getUTF8SequenceLength(first_byte);
        
        // P1-003 Fix: Validate seq_length before arithmetic to prevent integer overflow
        if (seq_length < 1 || seq_length > 4) {
            i++; // Skip invalid byte
            continue;
        }
        
        if (i + seq_length > text.length()) {
            break;
        }
        
        if (seq_length == 1) {
            // ASCII character
            char c = text[i];
            if (c >= 'A' && c <= 'Z') {
                metrics.english_character_count++;
            } else if (c >= 'a' && c <= 'z') {
                metrics.english_character_count++;
            } else if (c >= '0' && c <= '9') {
                metrics.digit_count++;
            } else if (c == '.' || c == ',' || c == '!' || c == '?' || c == ':' || c == ';') {
                metrics.punctuation_count++;
            }
            i++;
        } else {
            // Multi-byte UTF-8 sequence (likely Thai)
            uint32_t codepoint = utf8ToUnicode(
                reinterpret_cast<const uint8_t*>(&text[i]), seq_length);
            
            if (isThaiCharacter(static_cast<uint16_t>(codepoint))) {
                metrics.thai_character_count++;
                
                if (isThaiToneMark(static_cast<uint16_t>(codepoint))) {
                    metrics.has_tone_marks = true;
                    metrics.has_combining_chars = true;
                }
                
                if (isThaiVowelSign(static_cast<uint16_t>(codepoint))) {
                    metrics.has_vowel_marks = true;
                    metrics.has_combining_chars = true;
                }
            }
            
            i += seq_length;
        }
        
        metrics.character_count++;
    }
    
    // Estimate display width (Thai characters are typically wider)
    metrics.display_width_estimate = 
        metrics.english_character_count * 0.6 +
        metrics.thai_character_count * 1.0 +
        metrics.digit_count * 0.5 +
        metrics.punctuation_count * 0.3;
    
    return metrics;
}

bool ThaiTextConverter::separateMixedLanguageText(const std::string& mixed_text, 
                                                std::string& thai_part, std::string& english_part) {
    thai_part.clear();
    english_part.clear();
    
    if (mixed_text.empty()) {
        return false;
    }
    
    // Try to split on common separators
    std::vector<std::string> parts;
    std::stringstream ss(mixed_text);
    std::string part;
    
    // Try splitting on '/' first
    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            // Trim whitespace
            part.erase(0, part.find_first_not_of(" \t"));
            part.erase(part.find_last_not_of(" \t") + 1);
            if (!part.empty()) {
                parts.push_back(part);
            }
        }
    }
    
    if (parts.size() < 2) {
        // Try splitting on '|'
        parts.clear();
        ss.clear();
        ss.str(mixed_text);
        while (std::getline(ss, part, '|')) {
            if (!part.empty()) {
                part.erase(0, part.find_first_not_of(" \t"));
                part.erase(part.find_last_not_of(" \t") + 1);
                if (!part.empty()) {
                    parts.push_back(part);
                }
            }
        }
    }
    
    // Classify parts as Thai or English
    for (const auto& p : parts) {
        if (containsThaiCharacters(p)) {
            if (thai_part.empty()) {
                thai_part = p;
            }
        } else {
            if (english_part.empty()) {
                english_part = p;
            }
        }
    }
    
    return !thai_part.empty() || !english_part.empty();
}

std::string ThaiTextConverter::formatServiceName(const std::string& thai_name, 
                                               const std::string& english_name, 
                                               bool prefer_thai) {
    if (prefer_thai && !thai_name.empty()) {
        if (!english_name.empty()) {
            return thai_name + " (" + english_name + ")";
        } else {
            return thai_name;
        }
    } else if (!english_name.empty()) {
        if (!thai_name.empty()) {
            return english_name + " (" + thai_name + ")";
        } else {
            return english_name;
        }
    } else {
        return "ไม่ระบุชื่อสถานี"; // "Station name not specified"
    }
}

std::string ThaiTextConverter::truncateThaiText(const std::string& thai_text, 
                                               size_t max_characters, 
                                               bool add_ellipsis) {
    if (thai_text.empty() || max_characters == 0) {
        return "";
    }
    
    auto metrics = analyzeThaiText(thai_text);
    if (metrics.character_count <= max_characters) {
        return thai_text;
    }
    
    // Truncate character by character (UTF-8 aware)
    std::string result;
    size_t char_count = 0;
    size_t i = 0;
    
    while (i < thai_text.length() && char_count < max_characters) {
        uint8_t first_byte = static_cast<uint8_t>(thai_text[i]);
        int seq_length = getUTF8SequenceLength(first_byte);
        
        // P1-003 Fix: Validate seq_length before arithmetic to prevent integer overflow
        if (seq_length < 1 || seq_length > 4) {
            i++; // Skip invalid byte
            continue;
        }
        
        if (i + seq_length > thai_text.length()) {
            break;
        }
        
        if (char_count + 1 < max_characters || !add_ellipsis) {
            for (int j = 0; j < seq_length; j++) {
                result += thai_text[i + j];
            }
        }
        
        i += seq_length;
        char_count++;
    }
    
    if (add_ellipsis && char_count >= max_characters && i < thai_text.length()) {
        result += "...";
    }
    
    return result;
}

std::string ThaiTextConverter::convertArabicNumeralsToThai(const std::string& text) {
    std::string result = text;
    
    for (const auto& mapping : ARABIC_TO_THAI_NUMERALS) {
        size_t pos = 0;
        while ((pos = result.find(mapping.first, pos)) != std::string::npos) {
            result.replace(pos, 1, mapping.second);
            pos += mapping.second.length();
        }
    }
    
    return result;
}

std::string ThaiTextConverter::convertThaiNumeralsToArabic(const std::string& thai_text) {
    std::string result = thai_text;
    
    for (const auto& mapping : THAI_TO_ARABIC_NUMERALS) {
        size_t pos = 0;
        while ((pos = result.find(mapping.first, pos)) != std::string::npos) {
            result.replace(pos, mapping.first.length(), 1, mapping.second);
            pos += 1;
        }
    }
    
    return result;
}

double ThaiTextConverter::estimateThaiTextWidth(const std::string& thai_text, int font_size) {
    auto metrics = analyzeThaiText(thai_text);
    
    // Rough estimation based on character types and font size
    double base_width = font_size * 0.6; // Base character width
    double thai_multiplier = 1.2; // Thai characters are typically wider
    double english_multiplier = 0.6; // English characters are narrower
    
    double estimated_width = 
        (metrics.thai_character_count * base_width * thai_multiplier) +
        (metrics.english_character_count * base_width * english_multiplier) +
        (metrics.digit_count * base_width * 0.5) +
        (metrics.punctuation_count * base_width * 0.3);
    
    return estimated_width;
}

bool ThaiTextConverter::containsThaiCharacters(const std::string& text) {
    for (size_t i = 0; i < text.length(); ) {
        uint8_t first_byte = static_cast<uint8_t>(text[i]);
        int seq_length = getUTF8SequenceLength(first_byte);
        
        // P1-003 Fix: Validate seq_length before arithmetic to prevent integer overflow
        if (seq_length < 1 || seq_length > 4) {
            i++; // Skip invalid byte
            continue;
        }
        
        if (i + seq_length > text.length()) {
            break;
        }
        
        if (seq_length > 1) {
            uint32_t codepoint = utf8ToUnicode(
                reinterpret_cast<const uint8_t*>(&text[i]), seq_length);
            
            if (isThaiCharacter(static_cast<uint16_t>(codepoint))) {
                return true;
            }
        }
        
        i += seq_length;
    }
    
    return false;
}

ThaiTextConverter::ThaiDisplayProperties 
ThaiTextConverter::getRecommendedDisplayProperties(size_t text_length, const std::string& display_context) {
    ThaiDisplayProperties props;
    
    // Default properties
    props.font_family = "Noto Sans Thai";
    props.direction = ThaiTextDirection::LeftToRight;
    props.word_wrap = true;
    props.line_break_anywhere = false;
    props.text_color = "#333333";
    props.background_color = "transparent";
    
    // Adjust based on context
    if (display_context == "service_list") {
        props.font_size = 14;
        props.max_line_width = 200;
    } else if (display_context == "dls") {
        props.font_size = 12;
        props.max_line_width = 300;
        props.word_wrap = true;
    } else if (display_context == "epg") {
        props.font_size = 11;
        props.max_line_width = 250;
        props.line_break_anywhere = true;
    } else {
        // Default context
        props.font_size = 12;
        props.max_line_width = 200;
    }
    
    // Adjust font size based on text length
    if (text_length > 50) {
        props.font_size = std::max(10, props.font_size - 1);
    }
    
    return props;
}

// Private helper functions

bool ThaiTextConverter::isThaiCharacter(uint16_t codepoint) {
    return codepoint >= THAI_BLOCK_START && codepoint <= THAI_BLOCK_END;
}

bool ThaiTextConverter::isThaiToneMark(uint16_t codepoint) {
    return std::find(THAI_TONE_MARKS.begin(), THAI_TONE_MARKS.end(), codepoint) != THAI_TONE_MARKS.end();
}

bool ThaiTextConverter::isThaiVowelSign(uint16_t codepoint) {
    return std::find(THAI_VOWEL_SIGNS.begin(), THAI_VOWEL_SIGNS.end(), codepoint) != THAI_VOWEL_SIGNS.end();
}

int ThaiTextConverter::getUTF8SequenceLength(uint8_t first_byte) {
    if ((first_byte & 0x80) == 0) return 1;      // 0xxxxxxx
    if ((first_byte & 0xE0) == 0xC0) return 2;   // 110xxxxx
    if ((first_byte & 0xF0) == 0xE0) return 3;   // 1110xxxx
    if ((first_byte & 0xF8) == 0xF0) return 4;   // 11110xxx
    return 1; // Invalid sequence, treat as single byte
}

uint32_t ThaiTextConverter::utf8ToUnicode(const uint8_t* utf8_bytes, int length) {
    // P1-002 Fix: Add null pointer check
    if (!utf8_bytes) {
        return 0xFFFD;  // Unicode replacement character
    }
    
    // Validate length parameter
    if (length < 1 || length > 4) {
        return 0xFFFD;
    }
    
    if (length == 1) {
        return utf8_bytes[0];
    } else if (length == 2) {
        // Validate UTF-8 continuation byte (must be 10xxxxxx)
        if ((utf8_bytes[1] & 0xC0) != 0x80) {
            return 0xFFFD;
        }
        return ((utf8_bytes[0] & 0x1F) << 6) | (utf8_bytes[1] & 0x3F);
    } else if (length == 3) {
        // Validate UTF-8 continuation bytes
        if ((utf8_bytes[1] & 0xC0) != 0x80 || (utf8_bytes[2] & 0xC0) != 0x80) {
            return 0xFFFD;
        }
        return ((utf8_bytes[0] & 0x0F) << 12) | 
               ((utf8_bytes[1] & 0x3F) << 6) | 
               (utf8_bytes[2] & 0x3F);
    } else if (length == 4) {
        // Validate UTF-8 continuation bytes
        if ((utf8_bytes[1] & 0xC0) != 0x80 || 
            (utf8_bytes[2] & 0xC0) != 0x80 || 
            (utf8_bytes[3] & 0xC0) != 0x80) {
            return 0xFFFD;
        }
        return ((utf8_bytes[0] & 0x07) << 18) | 
               ((utf8_bytes[1] & 0x3F) << 12) | 
               ((utf8_bytes[2] & 0x3F) << 6) | 
               (utf8_bytes[3] & 0x3F);
    }
    return 0xFFFD;
}

std::vector<uint8_t> ThaiTextConverter::unicodeToUTF8(uint32_t codepoint) {
    std::vector<uint8_t> result;
    
    if (codepoint <= 0x7F) {
        result.push_back(static_cast<uint8_t>(codepoint));
    } else if (codepoint <= 0x7FF) {
        result.push_back(0xC0 | (codepoint >> 6));
        result.push_back(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        result.push_back(0xE0 | (codepoint >> 12));
        result.push_back(0x80 | ((codepoint >> 6) & 0x3F));
        result.push_back(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0x10FFFF) {
        result.push_back(0xF0 | (codepoint >> 18));
        result.push_back(0x80 | ((codepoint >> 12) & 0x3F));
        result.push_back(0x80 | ((codepoint >> 6) & 0x3F));
        result.push_back(0x80 | (codepoint & 0x3F));
    }
    
    return result;
}