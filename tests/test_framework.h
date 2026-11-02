#pragma once

#include "Token.h"
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <stdexcept>

inline std::ostream& operator<<(std::ostream& os, TokenType type) {
    return os << tokenTypeToString(type);
}

namespace test {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase>& getRegistry() {
    static std::vector<TestCase> registry;
    return registry;
}

inline bool registerTest(const std::string& name, std::function<void()> func) {
    getRegistry().push_back({name, std::move(func)});
    return true;
}

struct TestFailureException {
    std::string message;
    std::string file;
    int line;
};

inline int runAllTests() {
    int passed = 0;
    int failed = 0;
    auto& tests = getRegistry();

    std::cout << "=========================================================\n";
    std::cout << " Running Rose Compiler Test Suite (" << tests.size() << " test cases)\n";
    std::cout << "=========================================================\n";

    for (const auto& testCase : tests) {
        std::cout << "  [RUN] " << testCase.name << "... ";
        try {
            testCase.func();
            std::cout << "\033[32mPASSED\033[0m\n";
            passed++;
        } catch (const TestFailureException& e) {
            std::cout << "\033[31mFAILED\033[0m\n";
            std::cout << "        --> Assertion failed at " << e.file << ":" << e.line << "\n";
            std::cout << "        --> " << e.message << "\n";
            failed++;
        } catch (const std::exception& e) {
            std::cout << "\033[31mERROR\033[0m\n";
            std::cout << "        --> Exception: " << e.what() << "\n";
            failed++;
        } catch (...) {
            std::cout << "\033[31mERROR\033[0m\n";
            std::cout << "        --> Unknown exception thrown\n";
            failed++;
        }
    }

    std::cout << "=========================================================\n";
    if (failed == 0) {
        std::cout << "\033[32m ALL " << passed << " TESTS PASSED SUCCESSFULLY!\033[0m\n";
    } else {
        std::cout << "\033[31m TEST RUN COMPLETED: " << passed << " passed, " << failed << " failed.\033[0m\n";
    }
    std::cout << "=========================================================\n";

    return failed == 0 ? 0 : 1;
}

} // namespace test

#define TEST_CONCAT_IMPL(x, y) x##y
#define TEST_CONCAT(x, y) TEST_CONCAT_IMPL(x, y)

#define TEST_CASE(name) \
    static void TEST_CONCAT(test_func_, __LINE__)(); \
    static const bool TEST_CONCAT(test_reg_, __LINE__) = ::test::registerTest(name, TEST_CONCAT(test_func_, __LINE__)); \
    static void TEST_CONCAT(test_func_, __LINE__)()

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            throw ::test::TestFailureException{"Condition is false: " #cond, __FILE__, __LINE__}; \
        } \
    } while (0)

#define ASSERT_FALSE(cond) \
    do { \
        if (cond) { \
            throw ::test::TestFailureException{"Condition is true: " #cond, __FILE__, __LINE__}; \
        } \
    } while (0)

#define ASSERT_EQ(a, b) \
    do { \
        auto valA = (a); \
        auto valB = (b); \
        if (valA != valB) { \
            std::ostringstream ss; \
            ss << "Expected equality (" << #a << " == " << #b << "), actual: '" << valA << "' vs '" << valB << "'"; \
            throw ::test::TestFailureException{ss.str(), __FILE__, __LINE__}; \
        } \
    } while (0)

#define ASSERT_NE(a, b) \
    do { \
        auto valA = (a); \
        auto valB = (b); \
        if (valA == valB) { \
            std::ostringstream ss; \
            ss << "Expected inequality (" << #a << " != " << #b << "), both are: '" << valA << "'"; \
            throw ::test::TestFailureException{ss.str(), __FILE__, __LINE__}; \
        } \
    } while (0)
