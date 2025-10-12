/^ThaiServiceParser::MOTSlideShowInfo ThaiServiceParser::parseThaiMOTSlideShow/,/^}/ {
  /MOTSlideShowInfo slide_info;/,/return slide_info;/ {
    /if (!mot_data \|\| length < 8) {/,/}/ b
    /slide_info\.transport_id/,/slide_info\.content_size/ b
    /\/\/ Look for MOT extension/,/return slide_info;/ {
      /\/\/ Look for MOT extension/ {
        c\    // Look for MOT extension headers with Thai captions\
    size_t pos = MOT_MIN_HEADER_SIZE;\
    while (pos + 2 <= length) {  // Ensure we can safely read header type and length\
        uint8_t header_type = mot_data[pos];\
        uint8_t header_length = mot_data[pos + 1];\
        \
        // Validate header before processing\
        if (!validateMOTHeader(mot_data, length, pos, header_length)) {\
            break;  // Stop parsing on invalid header\
        }\
        \
        if (header_type == 0x25 && header_length > 2) { // Content Description\
            uint8_t charset_flag = mot_data[pos + 2];\
            slide_info.caption_charset = getCharacterSetFromFlag(charset_flag);\
            \
            // Triple-checked bounds (validated by validateMOTHeader)\
            if (pos + 3 + header_length <= length) {\
                const uint8_t* caption_data = &mot_data[pos + 3];\
                \
                // Prevent underflow: ensure header_length >= 1\
                if (header_length < 1) break;\
                size_t caption_length = header_length - 1;\
                \
                // Caption length enforcement\
                if (caption_length > 0 && caption_length <= MAX_MOT_CAPTION_LENGTH) {\
                    std::string full_caption = extractThaiText(caption_data, caption_length, slide_info.caption_charset);\
                    \
                    // Parse mixed language captions\
                    if (parseMixedLanguageContent(caption_data, caption_length, slide_info.caption_charset,\
                                                slide_info.caption_thai, slide_info.caption_english)) {\
                        // Mixed content successfully parsed\
                    } else {\
                        if (containsThaiCharacters(full_caption)) {\
                            slide_info.caption_thai = full_caption;\
                        } else {\
                            slide_info.caption_english = full_caption;\
                        }\
                    }\
                }\
            }\
        }\
        \
        // Safe position advancement (validated by validateMOTHeader)\
        pos += 2 + header_length;\
    }\
    \
    return slide_info;
      }
      d
    }
  }
}
