# End-to-End Testing Guide: DAB Announcement Support
## welle.io Thailand DAB+ Receiver

**Purpose:** E2E testing procedures for DAB announcement support  
**Date:** 2025-10-13  
**Status:** Complete integration - ready for testing

---

## Quick Test Checklist

### Unit Tests ✅
```bash
cd build/src/tests
./announcement_tests
# Expected: All tests passed (70+ assertions)
```

### Integration Verification ✅
1. Start welle-io: `./build/welle-io --channel 12B`
2. Check console for FIG messages:
   - `FIG 0/18: Service ... supports announcements`
   - `FIG 0/19: Announcement STARTED on cluster ...`
3. Enable announcements in Settings
4. Wait for announcement or trigger via ODR-DabMux
5. Verify audio switches and returns

### Success Criteria
- ✅ FIG 0/18/19 parsed and logged
- ✅ RadioController callbacks invoked
- ✅ Audio switches to announcement service
- ✅ Returns to original service when done
- ✅ Thai text displays correctly
- ✅ Buddhist calendar shows BE year

---

## Detailed Test Scenarios

See full testing guide in announcement-support-user-guide.md Section 10.

---

## Quick Debugging

### No FIG 0/18 detected?
```bash
./welle-io --channel 12B 2>&1 | grep "FIG 0"
```
If empty → Signal too weak or no announcement support in ensemble

### Audio doesn't switch?
Check console for: `"RadioController: No service found for subchannel XX"`
→ Subchannel ID mismatch

### Thai text garbled?
```bash
sudo apt-get install fonts-thai-tlwg
```

---

**Full guide:** docs/features/announcement-support-user-guide.md (Section 7-10)
