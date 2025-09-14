/*
 *    Thailand DAB+ Compliance Test Runner
 *    
 *    Simple test runner for Thailand DAB+ compliance tests
 */

#include "thailand_compliance_tests.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "welle.io Thailand DAB+ Compliance Test Suite" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    ThailandComplianceTests test_suite;
    
    bool all_passed = test_suite.runAllTests();
    
    if (all_passed) {
        std::cout << std::endl << "🎉 All tests PASSED! Thailand DAB+ compliance verified." << std::endl;
        return 0;
    } else {
        std::cout << std::endl << "❌ Some tests FAILED! Review implementation." << std::endl;
        return 1;
    }
}