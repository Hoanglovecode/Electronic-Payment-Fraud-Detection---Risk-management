#ifndef EPFD_TEST_FRAMEWORK_HPP
#define EPFD_TEST_FRAMEWORK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <sstream>

namespace epfd::test {

struct TestCase {
    std::string suite;
    std::string name;
    std::function<void()> func;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }

    void addTest(const std::string& suite, const std::string& name, std::function<void()> func) {
        tests_.push_back({suite, name, std::move(func)});
    }

    int runAll() {
        int passed = 0;
        int failed = 0;
        std::cout << "========================================================\n";
        std::cout << "           EPFD-RAS Test Suite Execution                \n";
        std::cout << "========================================================\n\n";

        auto start = std::chrono::high_resolution_clock::now();

        for (const auto& t : tests_) {
            std::cout << "[ RUN      ] " << t.suite << "." << t.name << "\n";
            try {
                t.func();
                std::cout << "[       OK ] " << t.suite << "." << t.name << "\n";
                passed++;
            } catch (const std::exception& e) {
                std::cerr << "[  FAILED  ] " << t.suite << "." << t.name 
                          << " | Error: " << e.what() << "\n";
                failed++;
            } catch (...) {
                std::cerr << "[  FAILED  ] " << t.suite << "." << t.name 
                          << " | Unknown exception thrown\n";
                failed++;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "\n========================================================\n";
        std::cout << "Total Tests: " << tests_.size() << " | Passed: " << passed 
                  << " | Failed: " << failed << " | Time: " << elapsed_ms << " ms\n";
        std::cout << "========================================================\n";

        return failed == 0 ? 0 : 1;
    }

private:
    std::vector<TestCase> tests_;
};

struct TestRegistrar {
    TestRegistrar(const std::string& suite, const std::string& name, std::function<void()> func) {
        TestRegistry::instance().addTest(suite, name, std::move(func));
    }
};

} // namespace epfd::test

#define EPFD_TEST(suite, name) \
    static void suite##_##name##_test_func(); \
    static ::epfd::test::TestRegistrar suite##_##name##_registrar(#suite, #name, suite##_##name##_test_func); \
    static void suite##_##name##_test_func()

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::ostringstream oss; \
            oss << "Assertion failed: (" #condition ") at " << __FILE__ << ":" << __LINE__; \
            throw std::runtime_error(oss.str()); \
        } \
    } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ(expected, actual) \
    do { \
        if (!((expected) == (actual))) { \
            std::ostringstream oss; \
            oss << "Assertion failed: expected [" << (expected) << "] but got [" << (actual) \
                << "] at " << __FILE__ << ":" << __LINE__; \
            throw std::runtime_error(oss.str()); \
        } \
    } while (0)

#define ASSERT_NEAR(val1, val2, abs_error) \
    do { \
        double diff = std::abs(static_cast<double>(val1) - static_cast<double>(val2)); \
        if (diff > (abs_error)) { \
            std::ostringstream oss; \
            oss << "Assertion failed: |" << (val1) << " - " << (val2) << "| = " << diff \
                << " > " << (abs_error) << " at " << __FILE__ << ":" << __LINE__; \
            throw std::runtime_error(oss.str()); \
        } \
    } while (0)

#endif // EPFD_TEST_FRAMEWORK_HPP
