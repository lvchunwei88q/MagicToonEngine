#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

inline int& GetFailedCount() { static int count = 0; return count; }

namespace test_detail
{
inline void Report(bool passed, bool fatal, const char* expression,
                   const char* file, int line, std::string_view message)
{
    if (passed)
    {
        if (!message.empty())
            std::cout << "PASS " << message << std::endl;
        return;
    }

    std::cerr << (fatal ? "FATAL: " : "FAILED: ");
    if (!message.empty())
        std::cerr << message << " | ";
    std::cerr << expression << " at " << file << ":" << line << std::endl;
    ++GetFailedCount();
    if (fatal)
        std::exit(1);
}
}

// Pass an optional description to log successful checks as well as failures.
#define EXPECT_TRUE(expr, ...) \
    test_detail::Report(!(!(expr)), false, #expr, __FILE__, __LINE__, std::string_view{__VA_ARGS__})
#define EXPECT_FALSE(expr, ...) \
    test_detail::Report(!(expr), false, #expr, __FILE__, __LINE__, std::string_view{__VA_ARGS__})
#define EXPECT_EQ(a, b, ...) \
    test_detail::Report(!((a) != (b)), false, #a " != " #b, __FILE__, __LINE__, std::string_view{__VA_ARGS__})
#define EXPECT_NE(a, b, ...) \
    test_detail::Report(!((a) == (b)), false, #a " == " #b, __FILE__, __LINE__, std::string_view{__VA_ARGS__})
#define ASSERT_TRUE(expr, ...) \
    test_detail::Report(!(!(expr)), true, #expr, __FILE__, __LINE__, std::string_view{__VA_ARGS__})
#define ASSERT_FALSE(expr, ...) \
    test_detail::Report(!(expr), true, #expr, __FILE__, __LINE__, std::string_view{__VA_ARGS__})
#define ASSERT_EQ(a, b, ...) \
    test_detail::Report(!((a) != (b)), true, #a " != " #b, __FILE__, __LINE__, std::string_view{__VA_ARGS__})
#define ASSERT_NE(a, b, ...) \
    test_detail::Report(!((a) == (b)), true, #a " == " #b, __FILE__, __LINE__, std::string_view{__VA_ARGS__})


template<typename Exception, typename Callable>
void ExpectThrows(Callable&& callable)
{
	bool threw = false;
	try {
		callable();
	}
	catch (const Exception&) {
		threw = true;
	}

	EXPECT_TRUE(threw);
}

// main function for running tests
inline int RunTests(void (*testFunc)()) 
{
    GetFailedCount() = 0;
    testFunc();
    int failedCount = GetFailedCount();
    if (failedCount > 0) {
        std::cerr << "Tests failed: " << failedCount << std::endl;
        return 1;
    } else {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    }
}
// define main function
#define TEST_MAIN(...) \
    int main() { \
        std::vector<void (*)()> funcs = { __VA_ARGS__ }; \
        size_t total = funcs.size(); \
        int totalFailed = 0; \
        \
        for (size_t i = 0; i < total; ++i) { \
            std::cout << "[" << (i + 1) << "/" << total << "] Running test..." << std::endl; \
            int failed = RunTests(funcs[i]); \
            if (failed > 0) { \
                std::cerr << "  -> Failed: " << failed << " error(s).\n"; \
                totalFailed += failed; \
            } else { \
                std::cout << "  -> Passed.\n"; \
            } \
        } \
        \
        std::cout << "\n==============================\n"; \
        if (totalFailed == 0) { \
            std::cout << "All " << total << " test(s) passed!\n"; \
            return 0; \
        } else { \
            std::cerr << "Total failures: " << totalFailed << "\n"; \
            return 1; \
        } \
    }