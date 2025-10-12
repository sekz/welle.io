# welle.io Thailand DAB+ CI/CD Security Testing

This directory contains GitHub Actions workflows for automated security testing and validation.

## Workflows

### `security-tests.yml` - Security Test Suite

Comprehensive security testing workflow that runs on every push and pull request affecting security-sensitive code.

#### Jobs

1. **security-tests** (Standard Build)
   - Builds security test suite in Debug mode
   - Runs all 23 security tests
   - Validates Phase 1 fixes (BUG-001, SECURITY-001, BUG-002)
   - Tests SecurityLogger functionality (Wave 2)
   - Tests MOT content_size validation (Wave 3)
   - **Duration:** ~5-7 minutes

2. **asan-tests** (AddressSanitizer)
   - Builds with `-fsanitize=address`
   - Detects memory errors: buffer overflows, use-after-free, leaks
   - Validates all 23 security tests under ASan
   - **Duration:** ~6-8 minutes
   - **Failure Criteria:** Any memory error causes immediate halt

3. **ubsan-tests** (UndefinedBehaviorSanitizer)
   - Builds with `-fsanitize=undefined`
   - Detects undefined behavior: integer overflow, alignment issues
   - Validates all 23 security tests under UBSan
   - **Duration:** ~5-7 minutes
   - **Failure Criteria:** Any undefined behavior causes immediate halt

4. **build-verification** (Production Builds)
   - Builds both `welle-cli` and `welle-io` in Release mode
   - Verifies executables link correctly with security fixes
   - Tests CLI help output for smoke testing
   - **Duration:** ~8-10 minutes

5. **security-report** (Report Generation)
   - Aggregates results from all jobs
   - Generates markdown security report
   - Posts report as PR comment (on pull requests)
   - **Duration:** ~1 minute

#### Triggers

**Push:**
- Branches: `master`, `main`, `develop`
- Paths:
  - `src/backend/thailand-compliance/**`
  - `src/tests/**`
  - `CMakeLists.txt`
  - `.github/workflows/security-tests.yml`

**Pull Request:**
- Same paths as push triggers

**Manual:**
- Can be triggered via GitHub Actions UI (workflow_dispatch)

#### Artifacts

All jobs upload artifacts for debugging:

- **security-test-results**: Standard test executable and logs (30 days)
- **asan-test-results**: ASan test executable (30 days)
- **ubsan-test-results**: UBSan test executable (30 days)
- **build-artifacts**: Production executables (7 days)
- **security-report**: Markdown report (90 days)

## Test Coverage

### Phase 1: Critical Security Fixes (15 tests)

**BUG-001: Buffer Overflow Protection (7 tests)**
- Null pointer handling
- Zero/minimum length validation
- Header bounds checking
- Integer overflow prevention
- Maximum size enforcement
- Malformed data handling

**SECURITY-001: Thread Safety (3 tests)**
- Concurrent DLS parsing (10 threads)
- Concurrent timestamp generation (100 threads)
- High concurrency stress (1000 operations)

**BUG-002: Pointer Safety (2 tests)**
- Const pointer correctness
- Non-ownership semantics

**General Memory Safety (3 tests)**
- Comprehensive null handling
- Empty buffer handling
- Large input handling (1MB)

### Wave 2: SecurityLogger (5 tests)

- Basic functionality and counters
- File logging verification
- Thread safety (50 threads × 20 events)
- Severity filtering
- MOT validation integration

### Wave 3: MOT Content Size Validation (3 tests)

- Excessive size rejection (100MB)
- Boundary conditions (16MB, 16MB+1, 1MB)
- Security logging verification

## Failure Handling

### Test Failures

If any test fails, the workflow:
1. Marks the job as failed (red X)
2. Uploads test artifacts for debugging
3. Generates security report showing which tests failed
4. Posts failure details to PR (if applicable)

### Sanitizer Failures

Sanitizers use aggressive failure modes:
- **ASan**: `halt_on_error=1` - stops immediately on first error
- **UBSan**: `halt_on_error=1:print_stacktrace=1` - stops with stack trace

This ensures no memory errors or undefined behavior goes unnoticed.

## Local Testing

You can run the same tests locally:

```bash
# Standard security tests
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_THAILAND_TESTS=ON
make test_security
./src/tests/test_security

# AddressSanitizer
mkdir build-asan && cd build-asan
cmake .. -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" \
  -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address" \
  -DBUILD_THAILAND_TESTS=ON
make test_security
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./src/tests/test_security

# UndefinedBehaviorSanitizer
mkdir build-ubsan && cd build-ubsan
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS="-fsanitize=undefined -fno-sanitize-recover=all -g" \
  -DCMAKE_C_FLAGS="-fsanitize=undefined -fno-sanitize-recover=all -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=undefined" \
  -DBUILD_THAILAND_TESTS=ON
make test_security
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./src/tests/test_security
```

## Badge Status

Add this to your README.md to show security test status:

```markdown
[![Security Tests](https://github.com/YOUR_USERNAME/welle.io-th/actions/workflows/security-tests.yml/badge.svg)](https://github.com/YOUR_USERNAME/welle.io-th/actions/workflows/security-tests.yml)
```

## Maintenance

### Adding New Tests

When adding new security tests:

1. Add test implementation to `src/tests/security_tests.cpp`
2. Add test declaration to `src/tests/security_tests.h`
3. Add test call in `runAllTests()` function
4. Update this README with test description
5. CI will automatically pick up and run the new test

### Updating Dependencies

If dependencies change, update the `apt-get install` lines in all jobs.

### Adjusting Timeouts

Default timeout is 60 minutes per job. Adjust if needed:

```yaml
jobs:
  security-tests:
    timeout-minutes: 30  # Adjust as needed
```

## Security Policy

All code changes affecting the Thailand DAB+ compliance module must:

1. ✅ Pass all 23 security tests
2. ✅ Pass AddressSanitizer validation
3. ✅ Pass UndefinedBehaviorSanitizer validation
4. ✅ Build successfully in Release mode
5. ✅ Receive security report approval

Any failures result in PR being blocked until resolved.

## Contact

For questions about security testing or CI/CD setup:
- Review Phase 2 documentation in `.claude/phases/phase-2.md`
- Check security test suite in `src/tests/security_tests.cpp`
- See SecurityLogger implementation in `src/backend/thailand-compliance/security_logger.{h,cpp}`
