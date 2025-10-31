# Emergency Warning System (EWS) Documentation

This directory contains documentation for ETSI TS 104 090 V1.1.2 compliance implementation in welle.io.

## Documents

### 1. [ETSI TS 104 090 Compliance Assessment](./etsi-ts-104-090-compliance-assessment.md)

**Purpose:** Comprehensive gap analysis between current welle.io implementation and ETSI EWS requirements

**Contents:**
- Executive summary of current compliance status (~30%)
- Detailed analysis of existing announcement support
- Gap analysis for 7 mandatory requirements
- Implementation roadmap (4-6 weeks estimated)
- Risk assessment

**Key Findings:**
- ✅ FIG 0/18/0/19 decoding exists
- ❌ Location code management missing (CRITICAL)
- ❌ Sleep/monitor duty cycle missing
- ❌ Alert stages support missing
- ❌ Other Ensemble (OE) signalling missing

**Target Audience:** Developers, project managers

---

### 2. [Test Execution Plan](./test-execution-plan.md)

**Purpose:** Step-by-step guide for executing ETSI TS 104 090 test suite

**Contents:**
- Test environment setup
- 7 test procedures (Tests 1-7)
- Expected results tables
- Automated testing scripts
- Compliance checklist

**Test Files Required:**
- EWS1.eti - EWS9.eti (download from ETSI docbox)
- Total size: ~450 MB
- Download link: https://docbox.etsi.org/Broadcast/Open/EWSv1_1.zip

**Target Audience:** QA engineers, testers

---

## Quick Start

### 1. Download Test Files

```bash
cd /home/seksan/workspace/welle.io
mkdir -p test-data/ews
cd test-data/ews

# Download from ETSI
wget https://docbox.etsi.org/Broadcast/Open/EWSv1_1.zip
unzip EWSv1_1.zip

# Verify checksums
sha256sum EWS*.eti
```

### 2. Build welle.io

```bash
cd /home/seksan/workspace/welle.io
./build-script.sh build
```

### 3. Run Test 1 (Basic)

```bash
# Using CLI
./build/src/welle-cli/welle-cli \
    -F rawfile,test-data/ews/EWS1.eti \
    -w 8080

# Using GUI
./build/src/welle-io/welle-io \
    --dump-file test-data/ews/EWS1.eti
```

### 4. Monitor Results

Open browser: http://localhost:8080

Expected behaviour (Test 1):
- At 0:00: Play "Service 5"
- At 1:05: Switch to "Alert 1"
- At 1:15: Return to "Service 5"

---

## Implementation Status

### Phase 1: Core EWS Support (NOT STARTED)

**Target:** 2-3 weeks

- [ ] Location code manager
  - [ ] Parse formats: `1255-4467-1352` and `Z1:91BB82`
  - [ ] Checksum validation
  - [ ] Location matching algorithm (NFF encoding)
- [ ] FIG 0/15 decoding (EWS ensemble flag)
- [ ] Alert stage support (IId 0-7)
- [ ] UI: Location code input dialog

### Phase 2: Sleep/Monitor Mode (NOT STARTED)

**Target:** 1-2 weeks

- [ ] FIG 0/10 time synchronization
- [ ] Minute-edge wakeup timer
- [ ] Low-power mode integration
- [ ] Alert monitoring in sleep

### Phase 3: OE Signalling (NOT STARTED)

**Target:** 1-2 weeks

- [ ] FIG 0/7 OE field extraction
- [ ] Other Ensemble alert monitoring
- [ ] 2-tuner support (for in-vehicle)

### Phase 4: Testing & Certification (NOT STARTED)

**Target:** 2-3 weeks

- [ ] Execute all 7 tests
- [ ] Bug fixes
- [ ] Documentation
- [ ] Optional: Apply for EWS Certification Mark

---

## Test Summary

| Test | Status | Blocking Issues |
|------|--------|----------------|
| Test 1: Set-up | ❌ FAIL | No location code input |
| Test 2: Area matching | ❌ FAIL | No location matching |
| Test 3: Alert stages | ❌ FAIL | No IId support |
| Test 4: OE alerts | ❌ FAIL | No OE signalling |
| Test 5: Sleep selection | ❌ FAIL | No sleep mode |
| Test 6: Sleep response | ❌ FAIL | No sleep mode |
| Test 7: Concurrent | ⚠️ PARTIAL | Missing concurrent logic |

**Overall Compliance:** **0% (0/7 tests passed)**

---

## Required Files

### From ETSI Docbox (Free Download)

**URL:** https://docbox.etsi.org/Broadcast/Open/EWSv1_1.zip

**Contents:**
```
EWSv1_1/
├── EWS1.eti          (30 MB, SHA256: b0b4787b...)
├── EWS2.eti          (30 MB, SHA256: 609a155c...)
├── EWS3.eti          (30 MB, SHA256: 0c73b2c7...)
├── EWS4.eti          (30 MB, SHA256: 42c41339...)
├── EWS5.eti          (75 MB, SHA256: f42c83a9...)
├── EWS6.eti          (75 MB, SHA256: a0918321...)
├── EWS7.eti          (75 MB, SHA256: 4108c9f8...)
├── EWS8.eti          (75 MB, SHA256: 5e76c55a...)
└── EWS9.eti          (75 MB, SHA256: e8554084...)
```

### From ETSI Store (Purchase Required)

**ETSI TS 104 090 V1.1.2** PDF document (already in `docs/ts_104090v010102p.pdf`)

---

## Key Concepts

### Location Codes

**Format 1 (Display):** `1255-4467-1352`
- Zone: `1` (0-41)
- L3: `255`
- L4: `4467`
- L5: `1352` (includes checksum)

**Format 2 (Hexadecimal):** `Z1:91BB82`
- Zone: `Z1`
- Location: `91BB82` (24-bit)

**NFF Encoding (4 levels):**
- L3: Regional (e.g., "Bangkok")
- L4: District (e.g., "Dusit")
- L5: Sub-district (e.g., "Thung Phaya Thai")
- L6: Precise location (100m radius)

### Alert Stages (IId)

| IId | Stage | Priority | Example |
|-----|-------|----------|---------|
| 0 | L1Start | Critical | "Tsunami warning issued" |
| 1 | L1Update | High | "Wave arriving in 10 minutes" |
| 2 | L1Repeat | High | "Repeat: Tsunami warning" |
| 3 | L1Critical | Critical | "Evacuate immediately" |
| 4 | L2Start | Medium | "Road traffic alert" |
| 5 | L2Update | Medium | "Traffic update" |
| 6 | L2Repeat | Medium | "Repeat: Road traffic" |
| 7 | Test | Low | "Test announcement" |

### Sleep/Monitor Duty Cycle

```
┌─────────┬──────────┬─────────┬──────────┬─────────┐
│ Sleep   │ Monitor  │ Sleep   │ Monitor  │ Sleep   │
│ 0:00    │ 0:55     │ 1:00    │ 1:55     │ 2:00    │
│         │ (5s)     │         │ (5s)     │         │
└─────────┴──────────┴─────────┴──────────┴─────────┘
           ↑                     ↑
           Wake at               Wake at
           minute edge           minute edge
```

- **Sleep:** Low power, no audio decode
- **Monitor:** Wake 5s before minute edge, check FIC for alerts
- **Sync:** Use FIG 0/10 ensemble time

---

## NBTC Thailand Considerations

### Thailand EWS Requirements

In addition to ETSI TS 104 090, Thailand NBTC may require:

1. **Thai Language Support:**
   - Location codes in Thai (e.g., "กรุงเทพฯ เขตดุสิต")
   - Alert messages in Thai character set (TIS-620)
   - Buddhist calendar dates (BE = CE + 543)

2. **Thailand Location Code Sets:**
   - Central region (กลาง)
   - Northern region (เหนือ)
   - Northeastern region (อีสาน)
   - Southern region (ใต้)
   - Bangkok metropolitan (กรุงเทพฯ)

3. **NBTC Frequency Plan:**
   - Channel 12B: 225.648 MHz (Bangkok)
   - Channel 12C: 227.360 MHz (Bangkok)
   - Channel 12D: 229.072 MHz (Bangkok)

### Integration Points

- Location code → Thai province/district mapping
- Alert type names in Thai
- UI text in Thai (already implemented in `th_TH.ts`)

---

## References

### Standards

- **ETSI TS 104 090 V1.1.2** (2025-02): Main EWS standard
- **ETSI TS 104 089**: EWS definition and rules
- **ETSI EN 300 401 V2.1.1**: DAB system specification
- **ETSI TS 103 461**: DAB receiver requirements

### External Links

- ETSI Docbox: https://docbox.etsi.org/Broadcast/Open/
- World DAB: https://www.worlddab.org/
- NBTC Thailand: https://www.nbtc.go.th/

### Internal Documentation

- [Phase 3 Wave 2 Docs](../phase3/wave2/) - Announcement feature implementation
- [CLAUDE.md](../../CLAUDE.md) - Project overview
- [Thailand Compliance](../../README_THAILAND.md) - NBTC requirements

---

## Contact

**Project:** welle.io Thailand Edition
**Repository:** https://github.com/AlbrechtL/welle.io
**Issues:** GitHub Issues

For EWS-specific questions, refer to:
- ETSI Technical Committee Broadcast
- World DAB Technical Committee

---

**Last Updated:** 2025-10-15
**Document Version:** 1.0

