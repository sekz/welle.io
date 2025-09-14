# Thai Font Resources

## Font Requirements for Thailand DAB+ Support

This directory should contain Thai fonts for proper Thai text rendering in welle.io.

### Recommended Fonts

1. **Noto Sans Thai** (Primary recommendation)
   - Download: https://fonts.google.com/noto/specimen/Noto+Sans+Thai
   - License: Open Font License (OFL)
   - File: `NotoSansThai-Regular.ttf`
   - Features: Complete Thai Unicode support, professional quality

2. **Alternative Fonts**
   - Sarabun (Thai Government font)
   - TH Sarabun New
   - Lato (with Thai support)

### Installation Instructions

1. Download `NotoSansThai-Regular.ttf` from Google Fonts
2. Place the font file in this directory
3. The CMake build system will automatically embed the font
4. Thai text will render properly in the UI components

### Font Integration

The Thai UI components reference fonts as:
```qml
Text {
    font.family: "Noto Sans Thai"
    text: "ข้อความภาษาไทย"
}
```

### Licensing Note

Due to distribution licensing, the actual font files are not included in this repository. Users must download and install appropriate Thai fonts according to their licensing terms.

For production deployment, ensure proper font licensing compliance.