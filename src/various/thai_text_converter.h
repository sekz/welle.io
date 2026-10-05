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

#ifndef __THAI_TEXT_CONVERTER_H
#define __THAI_TEXT_CONVERTER_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

/**
 * Thai Text Converter Utility
 * Handles Thai text processing, rendering, and display formatting
 * for DAB+ broadcasting applications in Thailand
 */
class ThaiTextConverter {
public:
    enum class ThaiTextDirection {
        LeftToRight,
        RightToLeft,
        TopToBottom
    };

    enum class ThaiTextCase {
        Original,
        Uppercase,
        Lowercase,
        TitleCase
    };

    struct ThaiTextMetrics {
        size_t character_count;
        size_t thai_character_count;
        size_t english_character_count;
        size_t digit_count;
        size_t punctuation_count;
        bool has_tone_marks;
        bool has_vowel_marks;
        bool has_combining_chars;
        double display_width_estimate;
    };

    struct ThaiDisplayProperties {
        std::string font_family;
        int font_size;
        ThaiTextDirection direction;
        bool word_wrap;
        bool line_break_anywhere;
        int max_line_width;
        std::string text_color;
        std::string background_color;
    };

    /**
     * Convert TIS-620 encoded Thai text to UTF-8
     * @param tis620_data TIS-620 encoded data
     * @param length Data length
     * @return UTF-8 encoded string
     */
    static std::string convertTIS620ToUTF8(const uint8_t* tis620_data, size_t length);

    /**
     * Convert UTF-8 Thai text to TIS-620 encoding
     * @param utf8_text UTF-8 encoded Thai text
     * @return TIS-620 encoded data
     */
    static std::vector<uint8_t> convertUTF8ToTIS620(const std::string& utf8_text);

    /**
     * Normalize Thai text for display
     * @param thai_text Input Thai text
     * @return Normalized Thai text
     */
    static std::string normalizeThaiText(const std::string& thai_text);

    /**
     * Split Thai text into words using Thai word segmentation
     * @param thai_text Input Thai text
     * @return Vector of Thai words
     */
    static std::vector<std::string> segmentThaiWords(const std::string& thai_text);

    /**
     * Format Thai text for display in limited width
     * @param thai_text Input text
     * @param max_width Maximum display width in characters
     * @return Formatted text with line breaks
     */
    static std::string formatThaiTextForDisplay(const std::string& thai_text, int max_width);

    /**
     * Get text metrics for Thai content
     * @param text Text to analyze
     * @return Thai text metrics
     */
    static ThaiTextMetrics analyzeThaiText(const std::string& text);

    /**
     * Convert Thai text case
     * @param thai_text Input text
     * @param text_case Desired case conversion
     * @return Converted text
     */
    static std::string convertThaiTextCase(const std::string& thai_text, ThaiTextCase text_case);

    /**
     * Validate Thai text structure
     * @param thai_text Text to validate
     * @return true if text structure is valid
     */
    static bool validateThaiTextStructure(const std::string& thai_text);

    /**
     * Extract Thai and English parts from mixed text
     * @param mixed_text Mixed language text
     * @param thai_part Output Thai text
     * @param english_part Output English text
     * @return true if separation successful
     */
    static bool separateMixedLanguageText(const std::string& mixed_text, 
                                        std::string& thai_part, std::string& english_part);

    /**
     * Generate display-friendly service name
     * @param thai_name Thai service name
     * @param english_name English service name
     * @param prefer_thai Prefer Thai over English
     * @return Formatted service name
     */
    static std::string formatServiceName(const std::string& thai_name, 
                                       const std::string& english_name, 
                                       bool prefer_thai = true);

    /**
     * Truncate Thai text to fit display
     * @param thai_text Input text
     * @param max_characters Maximum characters
     * @param add_ellipsis Add ellipsis if truncated
     * @return Truncated text
     */
    static std::string truncateThaiText(const std::string& thai_text, 
                                      size_t max_characters, 
                                      bool add_ellipsis = true);

    /**
     * Convert Thai numerals to Arabic numerals
     * @param thai_text Text containing Thai numerals
     * @return Text with Arabic numerals
     */
    static std::string convertThaiNumeralsToArabic(const std::string& thai_text);

    /**
     * Convert Arabic numerals to Thai numerals
     * @param text Text containing Arabic numerals
     * @return Text with Thai numerals
     */
    static std::string convertArabicNumeralsToThai(const std::string& text);

    /**
     * Estimate display width of Thai text
     * @param thai_text Input text
     * @param font_size Font size in pixels
     * @return Estimated width in pixels
     */
    static double estimateThaiTextWidth(const std::string& thai_text, int font_size = 12);

    /**
     * Check if text contains Thai characters
     * @param text Text to check
     * @return true if contains Thai characters
     */
    static bool containsThaiCharacters(const std::string& text);

    /**
     * Get recommended display properties for Thai text
     * @param text_length Length of text
     * @param display_context Display context (service list, DLS, etc.)
     * @return Recommended display properties
     */
    static ThaiDisplayProperties getRecommendedDisplayProperties(size_t text_length, 
                                                               const std::string& display_context);

    // Friend class for security testing
    friend class SecurityTests;

private:
    // Thai character ranges in Unicode
    static const uint16_t THAI_BLOCK_START = 0x0E00;
    static const uint16_t THAI_BLOCK_END = 0x0E7F;
    
    // Thai numeral mappings
    static const std::map<char, std::string> ARABIC_TO_THAI_NUMERALS;
    static const std::map<std::string, char> THAI_TO_ARABIC_NUMERALS;
    
    // Common Thai words for segmentation
    static const std::vector<std::string> THAI_COMMON_WORDS;
    
    // Thai tone marks and vowel signs
    static const std::vector<uint16_t> THAI_TONE_MARKS;
    static const std::vector<uint16_t> THAI_VOWEL_SIGNS;
    static const std::vector<uint16_t> THAI_COMBINING_CHARS;
    
    /**
     * Check if Unicode codepoint is Thai character
     * @param codepoint Unicode codepoint
     * @return true if Thai character
     */
    static bool isThaiCharacter(uint16_t codepoint);
    
    /**
     * Check if Unicode codepoint is Thai tone mark
     * @param codepoint Unicode codepoint
     * @return true if tone mark
     */
    static bool isThaiToneMark(uint16_t codepoint);
    
    /**
     * Check if Unicode codepoint is Thai vowel sign
     * @param codepoint Unicode codepoint
     * @return true if vowel sign
     */
    static bool isThaiVowelSign(uint16_t codepoint);
    
    /**
     * Get UTF-8 sequence length from first byte
     * @param first_byte First byte of UTF-8 sequence
     * @return Length of UTF-8 sequence (1-4 bytes)
     */
    static int getUTF8SequenceLength(uint8_t first_byte);
    
    /**
     * Convert UTF-8 sequence to Unicode codepoint
     * @param utf8_bytes UTF-8 byte sequence
     * @param length Length of sequence
     * @return Unicode codepoint
     */
    static uint32_t utf8ToUnicode(const uint8_t* utf8_bytes, int length);
    
    /**
     * Convert Unicode codepoint to UTF-8
     * @param codepoint Unicode codepoint
     * @return UTF-8 byte sequence
     */
    static std::vector<uint8_t> unicodeToUTF8(uint32_t codepoint);
};

#endif // __THAI_TEXT_CONVERTER_H