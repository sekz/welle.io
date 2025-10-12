/*
 *    Thailand DAB+ Security Test Runner
 *    
 *    Test runner for security validation suite
 *    Tests critical vulnerability fixes (BUG-001, SECURITY-001, BUG-002)
 */

#include "security_tests.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "welle.io Thailand DAB+ Security Test Suite" << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << "Validating Phase 1 critical security fixes" << std::endl;
    std::cout << std::endl;
    
    SecurityTests test_suite;
    
    bool all_passed = test_suite.runAllTests();
    
    std::cout << std::endl;
    if (all_passed) {
        std::cout << "🎉 All security tests PASSED!" << std::endl;
        std::cout << "   Phase 1 fixes validated successfully." << std::endl;
        std::cout << "   ✓ BUG-001: Buffer overflow protection working" << std::endl;
        std::cout << "   ✓ SECURITY-001: Thread safety validated" << std::endl;
        std::cout << "   ✓ BUG-002: Pointer safety verified" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some security tests FAILED!" << std::endl;
        std::cout << "   Review implementation and fix issues." << std::endl;
        return 1;
    }
}
