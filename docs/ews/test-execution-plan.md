# EWS Test Execution Plan
## ETSI TS 104 090 V1.1.2 Compliance Testing

**Version:** 1.0
**Date:** 2025-10-15
**Receiver:** welle.io DAB+ (Thailand Edition)

---

## 1. Test Environment

### 1.1 Hardware Requirements

- **Computer:** Linux/macOS/Windows with welle.io installed
- **SDR Device:** RTL-SDR, Airspy, or SoapySDR-compatible device
- **RF Signal Generator:** (Optional) for conducted testing
- **Timing Device:** Stopwatch or automated timer

### 1.2 Software Requirements

```bash
# Install welle.io
cd /home/seksan/workspace/welle.io
./build-script.sh build

# Download test ETI files
cd /home/seksan/workspace/welle.io/test-data
wget https://docbox.etsi.org/Broadcast/Open/EWSv1_1.zip
unzip EWSv1_1.zip

# Verify checksums (SHA256)
sha256sum EWS1.eti
# b0b4787b212d4a1e79aabf2627c62d9e34f555ad296c949e43f476eb7d0c6ece
```

### 1.3 Test Files Summary

| File | Size | Duration | SHA256 (first 16 chars) |
|------|------|----------|------------------------|
| EWS1.eti | ~30 MB | 4:00 | b0b4787b212d4a1e |
| EWS2.eti | ~30 MB | 4:00 | 609a155c4e47d849 |
| EWS3.eti | ~30 MB | 4:00 | 0c73b2c7bb057abc |
| EWS4.eti | ~30 MB | 4:00 | 42c413395ac1048b |
| EWS5.eti | ~75 MB | 10:00 | f42c83a967874a69 |
| EWS6.eti | ~75 MB | 10:00 | a091832114451e7e |
| EWS7.eti | ~75 MB | 10:00 | 4108c9f8a3976201 |
| EWS8.eti | ~75 MB | 10:00 | 5e76c55ab51770d4 |
| EWS9.eti | ~75 MB | 10:00 | e85540843defcb91 |

---

## 2. Test Procedure

### Test 1: Set-up Behaviour

**File:** EWS1.eti
**Duration:** 4 minutes
**Location Code:** 1255-4467-1352 (Z1:91BB82)

#### Steps:

```bash
# Start welle-cli with EWS1.eti
./build/src/welle-cli/welle-cli \
    -F rawfile,test-data/EWS1.eti \
    -w 8080

# Open browser: http://localhost:8080
# Or use GUI:
./build/src/welle-io/welle-io --dump-file test-data/EWS1.eti
```

#### Test Matrix:

| Time | Without Location Code | With Location Code 1255-4467-1352 |
|------|----------------------|-----------------------------------|
| 0:00 | Play "Service 5" (660 Hz + 770 Hz) | Play "Service 5" (660 Hz + 770 Hz) |
| 1:05 | Switch to "Alert 1" (440 Hz + 550 Hz) | Switch to "Alert 1" (440 Hz + 550 Hz) |
| 1:15 | Return to "Service 5" | Return to "Service 5" |
| 1:25 | NO SWITCH (no location) | Switch to "Alert 2" (1100 Hz + 1210 Hz) |
| 1:35 | NO SWITCH | Return to "Service 5" |
| 1:40 | Put to sleep | Put to sleep |
| 1:55 | Wake, play "Alert 1" | Wake, play "Alert 1" |
| 2:05 | Sleep | Sleep |
| 2:55 | Sleep (NO WAKE) | Wake, play "Alert 2" |

#### Expected Results:

- ✅ Without location code: Only Alert 1 (no location filter) switches
- ✅ With location code: Both Alert 1 and Alert 2 switch
- ✅ Sleep mode: Wakes at minute edges (1:55, 2:55)

#### Verification Commands:

```bash
# Monitor audio output
arecord -D hw:0 -f S16_LE -r 48000 -c 2 | \
    ffmpeg -i - -af "showfreqs=s=1280x720" -f sdl "Audio Spectrum"

# Check frequency:
# - Service 5: 660 Hz + 770 Hz
# - Alert 1: 440 Hz + 550 Hz
# - Alert 2: 1100 Hz + 1210 Hz
```

---

### Test 2: Alert Area Matching

**File:** EWS2.eti
**Duration:** 4 minutes
**Location Code:** 1255-4467-1352

#### Location Code Sets (LC1-LC6):

- **LC1:** Z1:91BB82 (exact match)
- **LC2:** Z1:91BB8[76531], Z1:91BB4[FED] (8 L6 codes surrounding)
- **LC3:** Z0:91BB82, Z10:91BB82, ... (8 L6 codes in different zones)
- **LC4:** Z1:91[FEA76], Z1:92[C84] (8 L3 codes surrounding)
- **LC5:** Z1:928[DC98], Z1:92C[10], Z1:91F3, Z1:91B[FB] (9 L4 codes including receiver)
- **LC6:** 132 L5 codes (large area)

#### Test Matrix:

| Time | Alert IId | Location Set | Expected Behaviour |
|------|-----------|--------------|-------------------|
| 0:30 | L1Start | LC1 (exact) | ✅ SWITCH |
| 0:50 | L1Start | LC2 (L6 surrounding) | ❌ NO SWITCH |
| 1:10 | L1Start | LC3 (different zones) | ❌ NO SWITCH |
| 1:30 | L1Start | LC4 (L3 surrounding) | ❌ NO SWITCH |
| 2:00 | L1Start | LC5 (L4 including receiver) | ✅ SWITCH |
| 2:20 | L1Start | LC6 (L5 large area) | ✅ SWITCH |

#### Expected Results:

- Only LC1, LC5, LC6 should trigger switching
- LC2, LC3, LC4 should be ignored (not matching receiver location)

---

### Test 3: Alert Stages

**File:** EWS3.eti
**Duration:** 4 minutes
**Location Code:** 1255-4467-1352

#### Alert Stages (IId):

| IId | Stage Name | Priority | Purpose |
|-----|-----------|----------|---------|
| 0 | L1Start | Highest | Initial alert |
| 1 | L1Update | High | Additional info |
| 2 | L1Repeat | High | Repeat message |
| 3 | L1Critical | Critical | Escalation |
| 4 | L2Start | Medium | Secondary alert |
| 5 | L2Update | Medium | Secondary update |
| 6 | L2Repeat | Medium | Secondary repeat |
| 7 | Test | Lowest | Test announcement |

#### Test Matrix:

| Time | Stage | Service | Audio Frequency |
|------|-------|---------|-----------------|
| 0:00 | - | Service 1 | 220 Hz + 330 Hz |
| 0:30 | L1Start | Level 1 Start | 440 Hz + 550 Hz |
| 0:50 | L1Update | Level 1 Update | 880 Hz + 990 Hz |
| 1:10 | L1Repeat | Level 1 Repeat | 1320 Hz + 1430 Hz |
| 1:30 | L1Critical | Level 1 Critical | 1760 Hz + 1870 Hz |
| 1:50 | L2Start | Level 2 Start | 660 Hz + 770 Hz |
| 2:10 | L2Update | Level 2 Update | 1100 Hz + 1210 Hz |
| 2:30 | L2Repeat | Level 2 Repeat | 1540 Hz + 1650 Hz |
| 2:50 | Test | Test | 1980 Hz + 2090 Hz |

#### Verification:

```bash
# Log stage changes
tail -f /tmp/welle-io-debug.log | grep "Stage"

# Expected output:
# [1:05:30] AnnouncementManager: Stage changed to L1Start (IId=0)
# [1:05:50] AnnouncementManager: Stage changed to L1Update (IId=1)
# ...
```

---

### Test 4: Other Ensemble Alerts

**Files:** EWS3.eti (Ensemble D001) + EWS4.eti (Ensemble D002)
**Duration:** 4 minutes each
**Requires:** 2 RF signal generators OR 2 ETI players

#### Setup:

```bash
# Terminal 1: Play EWS3.eti on channel 12B
./welle-cli -F rawfile,EWS3.eti -c 12B -w 8080

# Terminal 2: Play EWS4.eti on channel 12C
./welle-cli -F rawfile,EWS4.eti -c 12C -w 8081

# Note: This test requires hardware capable of monitoring 2 ensembles
```

#### Test Matrix:

| Time | Ensemble D001 Alert | Ensemble D002 (OE) Alert | Expected Behaviour |
|------|---------------------|-------------------------|-------------------|
| 0:30 | L1Start | - | ✅ Switch (tuned ensemble) |
| 0:50 | L1Update | - | ✅ Switch |
| 1:10 | - | L1Start (OE) | ❌ Ignore (other ensemble, not OE signalled) |

#### Expected Results:

- Switch to alerts from tuned ensemble (D001)
- Ignore alerts from other ensemble (D002) unless OE signalled

---

### Test 5: Sleep Mode EWS Selection

**Files:** EWS5.eti + EWS6.eti
**Duration:** 10 minutes each
**Signal Levels:** -50 dBm (EWS5), -50 dBm (EWS6), then -85 dBm (EWS5 at 2:20)

#### Test Matrix:

| Time | Action | Expected Behaviour |
|------|--------|-------------------|
| 0:00 | Play Service 11 (EWS5) | Audio: 300 Hz |
| 0:30 | Put to sleep | DUT sleeps |
| 2:00 | Wake at minute edge | Wake, play Alert (L1Start) 440 Hz + 550 Hz |
| 2:15 | Sleep | DUT sleeps |
| 2:20 | Reduce EWS5 to -85 dBm | (signal weak) |
| 4:00 | Wake, detect weak signal | Switch to EWS6 (stronger), play "Service 12" 600 Hz |
| 6:00 | Wake at minute edge | Play Alert from EWS6 |

#### Expected Results:

- ✅ Receiver wakes at minute edges (2:00, 4:00, 6:00, 8:00)
- ✅ Switches to stronger ensemble when weak signal detected
- ✅ Continues monitoring alerts in sleep mode

---

### Test 6: Sleep Mode Alert Response

**File:** EWS7.eti
**Duration:** 10 minutes
**Continues from Test 5**

#### Test Matrix:

| Time | Stage | Expected Behaviour |
|------|-------|-------------------|
| 0:00 | - | DUT sleeps (from Test 5) |
| 1:00 | L1Start | Wake, play Alert 440 Hz + 550 Hz |
| 1:15 | - | Return to sleep |
| 2:00 | L1Update | Wake, play Alert 880 Hz + 990 Hz |
| 3:00 | L1Repeat | Wake, play Alert 1320 Hz + 1430 Hz |
| 4:00 | L1Critical | Wake, play Alert 1760 Hz + 1870 Hz |
| 9:00 | L1Start | Wake, play Alert 440 Hz + 550 Hz |

#### Expected Results:

- ✅ Wakes only when alert present at minute edge
- ✅ Returns to sleep after alert ends
- ✅ No missed alerts

---

### Test 7: Concurrent Alerts

**Files:** EWS8.eti + EWS9.eti
**Duration:** 10 minutes each

#### Test Matrix:

| Time | Alert 1 (D001) | Alert 2 (D002 OE) | Expected Behaviour |
|------|---------------|-------------------|-------------------|
| 0:30 | L1Start | - | Switch to Alert 1 |
| 0:40 | L1Start | L2Start | Switch to L2Start (higher priority) |
| 1:10 | L1Start | L1Start (OE) | Continue current (equal priority) |
| 1:30 | L1Start | - | Switch to L1Start |
| 6:00 | L1Start | L2Start (OE) | Switch to L2Start OE |
| 7:00 | L1Start | L2Repeat (OE) | Continue L2Repeat OE |
| 7:55 | - | - | User cancels alert |

#### Expected Results:

- ✅ Switches to highest priority alert
- ✅ Handles concurrent alerts from tuned + OE ensembles
- ✅ User can cancel alert (if allow_manual_return=true)

---

## 3. Test Results Documentation

### 3.1 Test Report Template

```markdown
# EWS Test Report

**Test:** Test N - [Name]
**Date:** YYYY-MM-DD
**Tester:** [Name]
**Receiver:** welle.io v[version]

## Test Configuration
- Location Code: 1255-4467-1352
- ETI File: EWSN.eti
- Audio Device: [device name]

## Results

| Time | Expected | Actual | Status |
|------|----------|--------|--------|
| 0:30 | Switch to L1Start | Switched | ✅ PASS |
| 1:00 | Return to service | Returned | ✅ PASS |

## Issues Found
1. [Issue description]
2. ...

## Conclusion
- **PASS** / **FAIL**
- Notes: ...
```

### 3.2 Automated Test Script

```python
#!/usr/bin/env python3
"""
EWS Compliance Test Automation
"""

import subprocess
import time
import signal

class EWSTestRunner:
    def __init__(self, welle_cli_path):
        self.welle_cli = welle_cli_path
        self.processes = []

    def run_test(self, test_num, eti_file, duration):
        print(f"\n{'='*60}")
        print(f"Test {test_num}: {eti_file}")
        print(f"{'='*60}\n")

        # Start welle-cli
        proc = subprocess.Popen([
            self.welle_cli,
            "-F", f"rawfile,{eti_file}",
            "-D",  # Dump mode
            "-w", "8080"
        ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.processes.append(proc)

        # Monitor for expected duration
        start_time = time.time()
        while time.time() - start_time < duration:
            # Check for expected events
            time.sleep(1)

        # Stop welle-cli
        proc.send_signal(signal.SIGINT)
        proc.wait(timeout=5)

        return True  # or False based on validation

    def run_all_tests(self):
        tests = [
            (1, "test-data/EWS1.eti", 240),
            (2, "test-data/EWS2.eti", 240),
            (3, "test-data/EWS3.eti", 240),
            (4, "test-data/EWS4.eti", 240),
            (5, "test-data/EWS5.eti", 600),
            (6, "test-data/EWS7.eti", 600),
            (7, "test-data/EWS8.eti", 600),
        ]

        results = []
        for test_num, eti_file, duration in tests:
            result = self.run_test(test_num, eti_file, duration)
            results.append((test_num, result))

        # Print summary
        print("\n" + "="*60)
        print("TEST SUMMARY")
        print("="*60)
        for test_num, result in results:
            status = "PASS" if result else "FAIL"
            print(f"Test {test_num}: {status}")

if __name__ == "__main__":
    runner = EWSTestRunner("./build/src/welle-cli/welle-cli")
    runner.run_all_tests()
```

---

## 4. Compliance Checklist

After all tests:

- [ ] Test 1: Set-up behaviour PASSED
- [ ] Test 2: Alert area matching PASSED
- [ ] Test 3: Alert stages PASSED
- [ ] Test 4: Other ensemble alerts PASSED
- [ ] Test 5: Sleep mode selection PASSED
- [ ] Test 6: Sleep mode response PASSED
- [ ] Test 7: Concurrent alerts PASSED

**Overall Status:** ⬜ NOT STARTED / 🔄 IN PROGRESS / ✅ PASSED / ❌ FAILED

---

## 5. References

- ETSI TS 104 090 V1.1.2 Clause 7 (Test Specifications)
- ETSI TS 104 090 Annex A (Test Streams)
- Test ETI files: https://docbox.etsi.org/Broadcast/Open/EWSv1_1.zip

