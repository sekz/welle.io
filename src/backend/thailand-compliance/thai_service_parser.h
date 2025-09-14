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

#ifndef __THAI_SERVICE_PARSER_H
#define __THAI_SERVICE_PARSER_H

#include <string>
#include <vector>
#include <cstdint>
#include "charsets.h"

/**
 * Thai Service Information Parser for DAB+ Broadcasting
 * Handles parsing of FIG data with Thai character set support
 * Implements ETSI TS 101 756 with Thailand extensions
 */
class ThaiServiceParser {
public:
    struct ThaiServiceInfo {
        uint32_t service_id;
        std::string thai_label;
        std::string english_label;
        std::string programme_type_thai;
        std::string programme_type_english;
        std::string description_thai;
        std::string description_english;
        CharacterSet label_charset;
        bool emergency_support;
        bool traffic_support;
        bool mixed_language_support;
        uint8_t programme_type_code;
        uint16_t service_component_id;
    };

    struct ThaiDLSInfo {
        std::string text_thai;
        std::string text_english;
        CharacterSet charset;
        uint8_t segment_number;
        uint8_t toggle_flag;
        bool command_flag;
        std::string timestamp;
    };

    struct FIG1_Data {
        uint16_t service_id;
        uint8_t* label_data;
        uint8_t label_length;
        uint8_t charset_flag;
        uint16_t character_flag_field;
    };

    struct MOTSlideShowInfo {
        std::string caption_thai;
        std::string caption_english;
        std::string category_thai;
        uint16_t transport_id;
        uint32_t content_size;
        std::string content_type;
        CharacterSet caption_charset;
    };

    /**
     * Parse FIG 1 service label with Thai character set support
     * @param fig1_data FIG 1 data structure
     * @return Thai service information
     */
    static ThaiServiceInfo parseThaiService(const FIG1_Data& fig1_data);

    /**
     * Parse Dynamic Label Segment (DLS) with Thai text support
     * @param dls_data Raw DLS data buffer
     * @param length Length of DLS data
     * @return Thai DLS information
     */
    static ThaiDLSInfo parseThaiDLS(const uint8_t* dls_data, size_t length);

    /**
     * Parse MOT SlideShow header with Thai caption support
     * @param mot_data MOT header data
     * @param length Data length
     * @return MOT SlideShow information with Thai captions
     */
    static MOTSlideShowInfo parseThaiMOTSlideShow(const uint8_t* mot_data, size_t length);

    /**
     * Extract Thai text from service label data
     * @param label_data Raw label data
     * @param length Data length
     * @param charset Character set identifier
     * @return Extracted Thai text
     */
    static std::string extractThaiText(const uint8_t* label_data, size_t length, CharacterSet charset);

    /**
     * Parse mixed Thai-English content
     * @param data Mixed language data
     * @param length Data length
     * @param charset Character set
     * @param thai_text Output Thai text
     * @param english_text Output English text
     * @return true if parsing successful
     */
    static bool parseMixedLanguageContent(const uint8_t* data, size_t length, CharacterSet charset,
                                        std::string& thai_text, std::string& english_text);

    /**
     * Get programme type description in Thai
     * @param programme_type_code Programme type code (0-31)
     * @return Thai description of programme type
     */
    static std::string getProgrammeTypeThai(uint8_t programme_type_code);

    /**
     * Get programme type description in English
     * @param programme_type_code Programme type code (0-31)
     * @return English description of programme type
     */
    static std::string getProgrammeTypeEnglish(uint8_t programme_type_code);

    /**
     * Validate Thai text encoding
     * @param text Text to validate
     * @return true if text contains valid Thai characters
     */
    static bool isValidThaiText(const std::string& text);

    /**
     * Convert service information to display format
     * @param service_info Thai service information
     * @param prefer_thai Prefer Thai text over English
     * @return Formatted display string
     */
    static std::string formatServiceForDisplay(const ThaiServiceInfo& service_info, bool prefer_thai = true);

    /**
     * Extract character set from FIG data
     * @param charset_flag Character set flag from FIG
     * @return Character set enum
     */
    static CharacterSet getCharacterSetFromFlag(uint8_t charset_flag);

private:
    // Programme type descriptions in Thai
    static const std::vector<std::string> PROGRAMME_TYPES_THAI;
    
    // Programme type descriptions in English
    static const std::vector<std::string> PROGRAMME_TYPES_ENGLISH;
    
    // Thai character validation patterns
    static const uint16_t THAI_CHAR_RANGE_START = 0x0E00;
    static const uint16_t THAI_CHAR_RANGE_END = 0x0E7F;
    
    /**
     * Parse DLS segment
     * @param segment_data Segment data
     * @param segment_length Segment length
     * @return DLS text content
     */
    static std::string parseDLSSegment(const uint8_t* segment_data, size_t segment_length);
    
    /**
     * Decode Thai characters from TIS-620 encoding
     * @param tis620_data TIS-620 encoded data
     * @param length Data length
     * @return UTF-8 encoded Thai text
     */
    static std::string decodeTIS620ToUTF8(const uint8_t* tis620_data, size_t length);
    
    /**
     * Detect language in mixed content
     * @param text Text to analyze
     * @return true if text contains Thai characters
     */
    static bool containsThaiCharacters(const std::string& text);
};

#endif // __THAI_SERVICE_PARSER_H