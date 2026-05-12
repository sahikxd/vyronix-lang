#pragma once

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include "Error.hpp"

namespace vyronix {
namespace test {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

class TestRunner {
public:
    static TestRunner& getInstance() {
        static TestRunner instance;
        return instance;
    }

    void addTest(const std::string& name, std::function<void()> func) {
        tests_.push_back({name, std::move(func)});
    }

    bool runAll() {
        size_t passed = 0;
        size_t failed = 0;
        std::cout << "\n>>> Running VYRONIX Test Suite <<<\n" << std::endl;

        for (const auto& test : tests_) {
            std::cout << "[ RUN      ] " << test.name << std::endl;
            try {
                test.func();
                std::cout << "[       OK ] " << test.name << std::endl;
                passed++;
            } catch (const VyronixError& e) {
                std::cerr << "[  FAILED  ] " << test.name << "\n    Reason: " << e.what() << std::endl;
                failed++;
            } catch (const std::exception& e) {
                std::cerr << "[  FAILED  ] " << test.name << "\n    Reason (Std): " << e.what() << std::endl;
                failed++;
            } catch (...) {
                std::cerr << "[  FAILED  ] " << test.name << "\n    Reason: Unknown exception" << std::endl;
                failed++;
            }
        }

        std::cout << "\n>>> Test Summary <<<" << std::endl;
        std::cout << "Total:  " << tests_.size() << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;

        return failed == 0;
    }

private:
    TestRunner() = default;
    std::vector<TestCase> tests_;
};

#define VX_TEST(name) \
    static void name(); \
    static const bool name##_registered = []() { \
        vyronix::test::TestRunner::getInstance().addTest(#name, name); \
        return true; \
    }(); \
    static void name()

#define VX_ASSERT(cond) \
    if (!(cond)) throw std::runtime_error("Assertion failed: " #cond)

#define VX_ASSERT_THROWS(type, code) \
    try { \
        code; \
        throw std::runtime_error("Expected " #type " but no exception was thrown"); \
    } catch (const type& e) { \
        /* Success */ \
    }

} // namespace test
} // namespace vyronix
