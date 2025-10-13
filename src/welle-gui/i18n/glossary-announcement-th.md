# Thai-English Glossary for Announcement Feature
## welle.io Thailand DAB+ Receiver

**Document Version:** 1.0  
**Date:** 2025-10-13  
**Purpose:** Official terminology reference for Thai localization of DAB announcement features

---

## 1. Announcement Types (ETSI EN 300 401 Table 14)

### Official Translations (NBTC Compliant)

| English Term | Thai Translation | Phonetic | Priority | Usage Context |
|--------------|------------------|----------|----------|---------------|
| Alarm | เตือนภัย | dteuuan phai | 1 (Highest) | Emergency alerts, disaster warnings |
| Traffic | จราจร | ja-ra-jon | 2 | Road traffic updates, congestion |
| Transport Flash | ข่าวด่วนคมนาคม | khao duan kha-ma-na-khom | 3 | Public transport disruptions |
| Warning | คำเตือน | kham dteuuan | 4 | General warnings, advisory |
| News | ข่าว | khao | 5 | News bulletins, breaking news |
| Weather | สภาพอากาศ | sa-paap aa-kaat | 6 | Weather forecasts, storm warnings |
| Event | กิจกรรม | git-ja-gam | 7 | Event announcements |
| Special Event | กิจกรรมพิเศษ | git-ja-gam phi-set | 8 | Special event announcements |
| Programme Info | ข้อมูลรายการ | khor-muun rai-gaan | 9 | Programme schedule changes |
| Sport | กีฬา | gee-laa | 10 | Sports news, scores |
| Finance | การเงิน | gaan ngeun | 11 (Lowest) | Financial market updates |

### Alternative Translations (Informal/Colloquial)

| English | Formal Thai (Preferred) | Informal Thai | Notes |
|---------|------------------------|---------------|-------|
| Alarm | เตือนภัย | แจ้งเหตุฉุกเฉิน | Formal preferred for broadcasting |
| Traffic | จราจร | ข่าวรถติด | Colloquial more commonly used |
| Warning | คำเตือน | เตือน | Both acceptable |
| News | ข่าว | ข่าวสด | "ข่าวสด" means "live news" |
| Sport | กีฬา | ข่าวกีฬา | Add "ข่าว" for clarity |

---

## 2. UI Component Terminology

### 2.1 AnnouncementIndicator.qml Strings

| English | Thai Translation | Notes |
|---------|------------------|-------|
| Announcement Active | ประกาศฉุกเฉิน | Short, urgent |
| Announcement in Progress | กำลังเล่นประกาศ | More descriptive |
| Return to Service | กลับสู่รายการ | "Return to programme" |
| Return to Programme | กลับสู่รายการปกติ | More explicit |
| Duration | ระยะเวลา | Standard time duration |
| Elapsed Time | เวลาที่ผ่านไป | Time passed |
| Type | ประเภท | Category/type |
| Cluster | กลุ่ม | Group/cluster |
| Subchannel | ช่องย่อย | Sub-channel |
| Active | ใช้งานอยู่ | Currently active |
| Inactive | ไม่ใช้งาน | Not active |

### 2.2 AnnouncementSettings.qml Strings

| English | Thai Translation | Notes |
|---------|------------------|-------|
| Announcement Settings | ตั้งค่าประกาศ | Settings/preferences |
| Enable automatic announcement switching | เปิดใช้การสลับประกาศอัตโนมัติ | Master toggle |
| Announcement Types | ประเภทประกาศ | Category section header |
| Priority Threshold | ระดับความสำคัญขั้นต่ำ | Minimum priority level |
| Priority | ความสำคัญ | Priority level |
| Maximum Duration | ระยะเวลาสูงสุด | Max time limit |
| Allow manual return to service | อนุญาตให้กลับสู่รายการด้วยตนเอง | User override option |
| Quick Selection | เลือกด่วน | Quick preset selection |
| Critical Only | สำคัญเท่านั้น | Alarm/Warning only |
| All Types | ทุกประเภท | Enable all types |
| Clear All | ยกเลิกทั้งหมด | Disable all types |
| Reset to Defaults | รีเซ็ตค่าเริ่มต้น | Restore defaults |
| Apply | นำไปใช้ | Apply settings |
| Cancel | ยกเลิก | Cancel changes |
| Save | บันทึก | Save settings |
| Only announcements with priority %1 or higher | เฉพาะประกาศที่มีความสำคัญตั้งแต่ %1 ขึ้นไป | Description text |
| seconds | วินาที | Time unit |
| minutes | นาที | Time unit |
| Enabled | เปิดใช้ | Checkbox enabled |
| Disabled | ปิดใช้ | Checkbox disabled |

### 2.3 AnnouncementHistory.qml Strings

| English | Thai Translation | Notes |
|---------|------------------|-------|
| Announcement History | ประวัติประกาศ | History log |
| Filter by Type | กรองตามประเภท | Filter dropdown |
| Date Range | ช่วงวันที่ | Date range picker |
| From | จาก | Start date |
| To | ถึง | End date |
| Search Service | ค้นหารายการ | Search by service name |
| Clear Filters | ล้างตัวกรอง | Reset filters |
| Showing %1 of %2 announcements | แสดง %1 จาก %2 ประกาศ | Status text |
| No announcements found | ไม่พบประกาศ | Empty state |
| No announcements in history | ไม่มีประวัติประกาศ | Empty history |
| Statistics | สถิติ | Statistics section |
| Total Announcements | ประกาศทั้งหมด | Total count |
| This Session | เซสชันนี้ | Current session |
| All Time | ตลอดเวลา | All time stats |
| Most Frequent | บ่อยที่สุด | Most common type |
| Average Duration | ระยะเวลาเฉลี่ย | Mean duration |
| Last 24 Hours | 24 ชั่วโมงที่แล้ว | Recent period |
| Last 7 Days | 7 วันที่แล้ว | Recent period |
| Export to CSV | ส่งออกเป็น CSV | Export button |
| Export History | ส่งออกประวัติ | Export action |
| Export Successful | ส่งออกสำเร็จ | Success message |
| Export Failed | ส่งออกล้มเหลว | Error message |
| Max History Entries | รายการประวัติสูงสุด | Setting label |
| Auto-clear on exit | ล้างอัตโนมัติเมื่อปิดโปรแกรม | Setting checkbox |
| Clear History | ล้างประวัติ | Clear button |
| Confirm Clear History | ยืนยันการล้างประวัติ | Confirmation dialog |
| Are you sure? | คุณแน่ใจหรือไม่? | Confirmation question |

### 2.4 Table Headers

| English | Thai Translation | Notes |
|---------|------------------|-------|
| Time | เวลา | Timestamp column |
| Date | วันที่ | Date column |
| Type | ประเภท | Type column |
| Service | รายการ | Service name column |
| Duration | ระยะเวลา | Duration column |
| Cluster | กลุ่ม | Cluster ID column |
| Status | สถานะ | Status column |

---

## 3. Status Messages

### 3.1 Success Messages

| English | Thai Translation | Context |
|---------|------------------|---------|
| Announcement started: %1 | เริ่มประกาศ: %1 | Toast notification |
| Announcement ended | สิ้นสุดประกาศ | Toast notification |
| Returned to service | กลับสู่รายการแล้ว | Toast notification |
| Settings saved | บันทึกการตั้งค่าแล้ว | Save confirmation |
| Export successful | ส่งออกสำเร็จ | Export confirmation |
| Switched to announcement | สลับไปยังประกาศแล้ว | Switch confirmation |

### 3.2 Error Messages

| English | Thai Translation | Context |
|---------|------------------|---------|
| Announcement not available | ประกาศไม่พร้อมใช้งาน | Service error |
| Failed to switch to announcement | ไม่สามารถสลับไปยังประกาศได้ | Switch error |
| Subchannel not found | ไม่พบช่องย่อย | Tuning error |
| Service does not support announcements | รายการนี้ไม่รองรับประกาศ | Feature unavailable |
| Export failed: %1 | ส่งออกล้มเหลว: %1 | Export error |
| Invalid date range | ช่วงวันที่ไม่ถูกต้อง | Validation error |
| No data to export | ไม่มีข้อมูลสำหรับส่งออก | Export validation |

### 3.3 Warning Messages

| English | Thai Translation | Context |
|---------|------------------|---------|
| Announcement type disabled | ประเภทประกาศถูกปิดใช้งาน | User preference |
| Maximum duration exceeded | เกินระยะเวลาสูงสุด | Timeout warning |
| Priority too low | ความสำคัญต่ำเกินไป | Priority filter |
| Announcement will be skipped | จะข้ามประกาศนี้ | Skip notification |

---

## 4. Time & Date Formatting

### 4.1 Time Units

| English | Thai | Abbreviation |
|---------|------|--------------|
| seconds | วินาที | วิ. |
| minutes | นาที | น. |
| hours | ชั่วโมง | ชม. |
| days | วัน | วัน |
| weeks | สัปดาห์ | สัปดาห์ |

### 4.2 Date Format (Buddhist Era)

**Format String:** `"วันที่ %d %M พ.ศ. %Y เวลา %H:%m:%s"`

**Example:**
- CE: 2025-10-13 14:30:00
- BE: วันที่ 13 ตุลาคม พ.ศ. 2568 เวลา 14:30:00

**Thai Month Names:**
1. มกราคม (Makkaraakom) - January
2. กุมภาพันธ์ (Kumphaphan) - February
3. มีนาคม (Meenaakom) - March
4. เมษายน (Mesayon) - April
5. พฤษภาคม (Phruetsaphaakom) - May
6. มิถุนายน (Mithunayon) - June
7. กรกฎาคม (Karakadaakom) - July
8. สิงหาคม (Singhaakom) - August
9. กันยายน (Kanyayon) - September
10. ตุลาคม (Tulaakom) - October
11. พฤศจิกายน (Phruetsajikayon) - November
12. ธันวาคม (Thanwaakom) - December

**Day of Week:**
- วันจันทร์ (Chan) - Monday
- วันอังคาร (Angkhan) - Tuesday
- วันพุธ (Phut) - Wednesday
- วันพฤหัสบดี (Pharuehatsabodee) - Thursday
- วันศุกร์ (Suk) - Friday
- วันเสาร์ (Sao) - Saturday
- วันอาทิตย์ (Aathit) - Sunday

### 4.3 Relative Time

| English | Thai | Usage |
|---------|------|-------|
| just now | เมื่อสักครู่ | <1 minute ago |
| 1 minute ago | 1 นาทีที่แล้ว | 1 minute |
| %d minutes ago | %d นาทีที่แล้ว | 2-59 minutes |
| 1 hour ago | 1 ชั่วโมงที่แล้ว | 1 hour |
| %d hours ago | %d ชั่วโมงที่แล้ว | 2-23 hours |
| yesterday | เมื่อวาน | 1 day ago |
| %d days ago | %d วันที่แล้ว | 2+ days |

---

## 5. NBTC Official Terminology

### 5.1 Broadcasting Terms (from NBTC Regulations)

| English Term | NBTC Official Thai | Document Reference |
|--------------|-------------------|-------------------|
| Digital Audio Broadcasting | การออกอากาศเสียงดิจิทัล | ผว. 104-2567 |
| DAB+ | ดีเอบีพลัส | ผว. 104-2567 |
| Emergency Alert | การแจ้งเตือนฉุกเฉิน | พ.ร.บ. ป้องกันและบรรเทาสาธารณภัย |
| Traffic Information | ข้อมูลจราจร | กฎกระทรวงกำหนดระเบียบว่าด้วยการจราจร |
| Multiplex | มัลติเพล็กซ์ | ผว. 104-2567 |
| Service Component | ส่วนประกอบรายการ | ผว. 104-2567 |
| Subchannel | ช่องย่อย | ผว. 104-2567 |
| Ensemble | ชุดรายการ | ผว. 104-2567 |

### 5.2 Emergency Categories (Thailand Civil Defense)

| English | Thai Official Term | Authority |
|---------|-------------------|-----------|
| Disaster Warning | การเตือนภัยพิบัติ | กรมป้องกันและบรรเทาสาธารณภัย |
| Flood Alert | เตือนภัยน้ำท่วม | กรมอุตุนิยมวิทยา |
| Storm Warning | เตือนภัยพายุ | กรมอุตุนิยมวิทยา |
| Earthquake Alert | เตือนภัยแผ่นดินไหว | กรมอุตุนิยมวิทยา |
| Tsunami Warning | เตือนภัยคลื่นยักษ์ | กรมอุตุนิยมวิทยา |
| Air Quality Alert | เตือนภัยคุณภาพอากาศ | กรมควบคุมมลพิษ |

---

## 6. Typography & Layout Guidelines

### 6.1 Font Recommendations

**Primary Font (GUI):**
- TH Sarabun New (Open source, TLWG)
- Noto Sans Thai (Google Fonts)
- Leelawadee UI (Windows system font)

**Font Sizes (Thai text requires ~10% larger than English):**
- Heading 1: 20pt (vs 18pt English)
- Heading 2: 16pt (vs 14pt English)
- Body text: 14pt (vs 12pt English)
- Small text: 12pt (vs 10pt English)

### 6.2 Text Length Multipliers

Thai text is typically 20-40% longer than English equivalents:

| English Length | Thai Multiplier | Example |
|----------------|-----------------|---------|
| 1-10 chars | 1.4x | "Alarm" (5) → "เตือนภัย" (7) |
| 11-30 chars | 1.3x | "Announcement" (12) → "ประกาศฉุกเฉิน" (15) |
| 31+ chars | 1.2x | Sentences |

**UI Implications:**
- Increase button widths by 30%
- Use multi-line labels for long strings
- Increase table column widths by 25%
- Consider horizontal scrolling for narrow views

### 6.3 Line Height

Thai script requires more vertical space due to tone marks and vowels above/below:

- Minimum line height: 1.6 (vs 1.4 for English)
- Recommended: 1.8 for comfortable reading

---

## 7. Cultural & Linguistic Notes

### 7.1 Politeness Levels

Thai has multiple politeness levels. For broadcasting and UI:

**Formal (ราชาศัพท์ - Royal vocabulary):** Not used in DAB context  
**Polite (ภาษากลาง - Central polite):** **USE THIS** - Standard for public broadcasting  
**Informal (ภาษาพูด - Spoken informal):** Avoid in UI

**Examples:**
- Polite: "กรุณา" (please) - USE
- Informal: "ช่วย" (help) - AVOID in UI

### 7.2 Compound Words

Thai often creates compound words without spaces:

- "ประกาศฉุกเฉิน" (announcement + emergency) = Emergency announcement
- "ข่าวด่วน" (news + urgent) = Breaking news
- "พยากรณ์อากาศ" (forecast + weather) = Weather forecast

**UI Consideration:** These compounds don't break naturally. Use zero-width space (U+200B) if line breaks needed:
```
ประกาศฉุก​เฉิน  (with U+200B)
```

### 7.3 Number Formatting

**Thai uses Western Arabic numerals (0-9), NOT Thai numerals (๐-๙)** in DAB broadcasting context.

**Correct:**
- Priority: 1, 2, 3, 4...
- Duration: 30 วินาที (30 seconds)

**Incorrect (Don't use Thai numerals in UI):**
- Priority: ๑, ๒, ๓, ๔...
- Duration: ๓๐ วินาที

**Exception:** Official government documents may use Thai numerals, but modern digital interfaces use Western numerals.

### 7.4 Time Format

**24-hour clock preferred:**
- 14:30:00 (NOT 2:30 PM)
- Thai: "14:30 น." (น. = นาฬิกา = o'clock)

**12-hour format (if needed):**
- 14:30 = "บ่าย 2:30 น." (afternoon 2:30)
- 09:00 = "เช้า 9:00 น." (morning 9:00)
- 21:00 = "ค่ำ 9:00 น." (evening 9:00)

---

## 8. Translation Quality Checklist

### Before Submitting Translations:

- [ ] Used official NBTC terminology where applicable
- [ ] Checked against existing th_TH.ts for consistency
- [ ] Verified Buddhist Era (BE) year conversion (CE + 543)
- [ ] Used polite/formal register (ภาษากลาง)
- [ ] Avoided informal/slang terms
- [ ] Tested text length fits UI components (30% buffer)
- [ ] Verified line breaks work naturally
- [ ] Used Western Arabic numerals (0-9), not Thai numerals (๐-๙)
- [ ] Checked 24-hour time format
- [ ] Ensured font supports all Thai characters (tone marks, vowels)
- [ ] Tested on 1366x768 resolution (minimum target)
- [ ] Verified with native Thai speaker (if possible)

---

## 9. References

### Official Documents

1. **NBTC Broadcasting Regulations**
   - ผว. 104-2567: Thailand DAB+ Technical Standards
   - Website: https://www.nbtc.go.th/

2. **Royal Institute Dictionary**
   - Official Thai language authority
   - Website: https://www.royin.go.th/

3. **ETSI EN 300 401**
   - European DAB standard (English source)
   - Announcement types (Table 14, Section 8.1.6)

### Typography Resources

1. **TLWG Fonts Project**
   - Package: fonts-thai-tlwg
   - License: GPL, free for commercial use

2. **Google Fonts - Noto Sans Thai**
   - Free, open source
   - Excellent Unicode coverage

### Community Resources

1. **Thai Language Stack Exchange**
   - Technical translation questions
   - https://thai.stackexchange.com/

2. **NBTC Public Forum**
   - Broadcasting terminology clarifications
   - https://www.nbtc.go.th/forum/

---

## 10. Revision History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2025-10-13 | Initial glossary for Wave 2B | Thailand Localization Team |

---

**Document Status:** FINAL  
**Approved by:** UI/UX Design Team, Thailand Broadcasting Standards  
**Next Review:** 2025-12-01
