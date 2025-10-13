# DAB Alarm Announcement Switching - Receiver Implementation Checklist

## ข้อกำหนดบังคับตาม ETSI EN 300 401 V2.1.1

### 1. ตรวจสอบ Al Flag ใน FIG 0/0 เสมอ

```
- [ ] Parse FIG 0/0 และเก็บค่า Al flag (bit 0 ของ byte ที่มี Al flag)
- [ ] Update Al flag status ทุกครั้งที่ได้รับ FIG 0/0 ใหม่
- [ ] เก็บ state: alarm_enabled = (Al flag == 1)
```

### 2. จัดการ Alarm Announcement (Cluster ID = 0xFF)

**เมื่อได้รับ FIG 0/19 หรือ FIG 0/26:**

```
IF Cluster_ID == 0xFF (255):
    IF alarm_enabled == TRUE (Al flag = 1):
        - [ ] MUST switch ไปยัง SubChId ที่ระบุทันที
        - [ ] IGNORE user preferences ทั้งหมด
        - [ ] OVERRIDE current service selection
        - [ ] เก็บ original service เพื่อกลับหลัง alarm จบ
        - [ ] แสดง alarm indicator บน UI
        - [ ] เล่น audio จาก alarm subchannel
    ELSE IF alarm_enabled == FALSE (Al flag = 0):
        - [ ] IGNORE announcement นี้ทั้งหมด
        - [ ] ไม่ต้อง switch
        - [ ] ทำงานปกติต่อ
```

### 3. ตรวจสอบการสิ้นสุด Alarm

```
- [ ] ตรวจจับเมื่อ FIG 0/19/0/26 มี ASw flags = 0x0000
- [ ] หรือเมื่อหยุดได้รับ FIG 0/19/0/26 นานกว่า threshold (เช่น 5 วินาที)
- [ ] Return to original service โดยอัตโนมัติ
- [ ] ลบ alarm indicator
```

### 4. จัดการ Announcement อื่นๆ (Cluster ID != 0xFF)

```
IF Cluster_ID != 0xFF:
    - [ ] ใช้กลไกปกติ (ตาม FIG 0/18, user settings)
    - [ ] ให้ผู้ใช้เลือกได้ว่าจะ switch หรือไม่
    - [ ] ไม่บังคับ switch
```

### 5. Priority Handling

```
Priority order (สูงสุด → ต่ำสุด):
1. Alarm Announcement (Cluster ID 0xFF, Al flag = 1) → MUST switch
2. User-selected announcements (other types) → MAY switch
3. Current service → Continue playing
```

### 6. Implementation Notes

```
- [ ] Al flag อยู่ใน FIG 0/0 ซึ่งส่งทุก 96ms
- [ ] Alarm ใช้ Cluster ID = 0xFF เท่านั้น (ค่าอื่นไม่ใช่ alarm)
- [ ] FIG 0/18 ไม่ใช้ bit 0 สำหรับ alarm (bit 0 = 0 เสมอ)
- [ ] Repetition rate: 10 Hz (5 วินาทีแรก) → 1 Hz (ongoing)
```

## Pseudo-code

```python
class DAB_Receiver:
    def __init__(self):
        self.alarm_enabled = False
        self.current_service = None
        self.original_service = None
        self.in_alarm_mode = False
    
    def on_receive_fig_0_0(self, fig_0_0):
        # Parse Al flag from FIG 0/0
        self.alarm_enabled = fig_0_0.get_al_flag()
    
    def on_receive_fig_0_19_or_0_26(self, announcement):
        cluster_id = announcement.cluster_id
        asw_flags = announcement.asw_flags
        subchannel = announcement.subchannel_id
        
        # Check if this is Alarm
        if cluster_id == 0xFF:
            if self.alarm_enabled:
                # MANDATORY SWITCH
                if not self.in_alarm_mode:
                    self.original_service = self.current_service
                    self.in_alarm_mode = True
                
                self.switch_to_subchannel(subchannel)
                self.show_alarm_indicator()
            else:
                # IGNORE
                pass
        else:
            # Regular announcement - check user preferences
            if self.user_wants_announcement(asw_flags):
                self.switch_to_subchannel(subchannel)
        
        # Check for end of announcement
        if asw_flags == 0x0000:
            self.end_announcement()
    
    def end_announcement(self):
        if self.in_alarm_mode:
            self.switch_to_service(self.original_service)
            self.hide_alarm_indicator()
            self.in_alarm_mode = False
```

## Testing Checklist

```
Test Case 1: Al flag = 1, Alarm announcement active
- [ ] Receiver MUST switch immediately
- [ ] IGNORE user preferences
- [ ] Show alarm indicator

Test Case 2: Al flag = 0, Alarm announcement active
- [ ] Receiver MUST NOT switch
- [ ] Continue current service

Test Case 3: Alarm ends (ASw flags = 0x0000)
- [ ] Return to original service automatically

Test Case 4: Regular announcement (Cluster ID != 0xFF)
- [ ] Follow normal announcement rules
- [ ] Respect user preferences

Test Case 5: Al flag changes from 1 → 0 during alarm
- [ ] Stop alarm processing
- [ ] Return to normal operation
```

## Critical Points ⚠️

```
❌ ห้าม:
- ให้ผู้ใช้ disable alarm (เมื่อ Al flag = 1)
- ใช้ Cluster ID อื่นนอกจาก 0xFF สำหรับ alarm
- ตรวจสอบ FIG 0/18 bit 0 สำหรับ alarm support

✅ ต้องทำ:
- ตรวจสอบ Al flag ใน FIG 0/0 เสมอ
- Switch ทันทีเมื่อได้รับ alarm (ถ้า Al flag = 1)
- กลับ service เดิมหลัง alarm จบ
- Ignore alarm เมื่อ Al flag = 0
```

## Quick Reference

```
Per ETSI EN 300 401 V2.1.1:

FIG 0/0:  Al flag (bit 1 after CN) → Enable/Disable alarm ensemble-wide
FIG 0/18: Cluster Id (8 bits, Figure 42) → ASu flags + cluster assignments
FIG 0/19: Cluster Id (8 bits, Figure 43) → 0xFF (255) = EXCLUSIVE for Alarm announcements
FIG 0/26: OE Cluster Id (8 bits, Figure 45) → 0xFF (255) = EXCLUSIVE for Alarm announcements

⚠️  IMPORTANT: Cluster Id = "1111 1111" (0xFF/255) SHALL be used EXCLUSIVELY for all Alarm announcements

Al flag = 1 → MUST switch (MANDATORY)
Al flag = 0 → MUST ignore (MANDATORY)
```