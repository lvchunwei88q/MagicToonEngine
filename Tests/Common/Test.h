// Test.h
// The test database is used for some functional testing

#include <iostream>
#include <vector>
#include <string>

#ifndef LOADED_DLL_H
#define LOADED_DLL_H

#include <stdexcept>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace EngineTest {

    class LoadedDll {
    public:
        explicit LoadedDll(const std::string& dllPath) {
#ifdef _WIN32
            // Convert a UTF-8 string to Windows UTF-16 wide characters
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, dllPath.c_str(), (int)dllPath.size(), NULL, 0);
            std::wstring wPath(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, dllPath.c_str(), (int)dllPath.size(), &wPath[0], size_needed);
            
            handle = LoadLibraryW(wPath.c_str());
            if (!handle) {
                throw std::runtime_error("LoadLibraryW failed with Windows error " + std::to_string(GetLastError()));
            }
#else
            // Linux/macOS Usse dlopen
            handle = dlopen(dllPath.c_str(), RTLD_LAZY);
            if (!handle) {
                throw std::runtime_error("dlopen failed: " + std::string(dlerror()));
            }
#endif
        }

        ~LoadedDll() {
#ifdef _WIN32
            if (handle) FreeLibrary(handle);
#else
            if (handle) dlclose(handle);
#endif
        }

        LoadedDll(const LoadedDll&) = delete;
        LoadedDll& operator=(const LoadedDll&) = delete;

        LoadedDll(LoadedDll&& other) noexcept : handle(other.handle) {
            other.handle = nullptr;
        }

        LoadedDll& operator=(LoadedDll&& other) noexcept {
            if (this != &other) {
                Release();
                handle = other.handle;
                other.handle = nullptr;
            }
            return *this;
        }

        // A general way to get symbols (function pointers)
        void* GetSymbol(const char* symbolName) const {
#ifdef _WIN32
            return reinterpret_cast<void*>(GetProcAddress(handle, symbolName));
#else
            return dlsym(handle, symbolName);
#endif
        }

#ifdef _WIN32
        HMODULE Get() const { return handle; }
#else
        void* Get() const { return handle; }
#endif

    private:
        void Release() {
#ifdef _WIN32
            if (handle) FreeLibrary(handle);
#else
            if (handle) dlclose(handle);
#endif
            handle = nullptr;
        }

#ifdef _WIN32
        HMODULE handle = nullptr;
#else
        void* handle = nullptr;
#endif
    };

}
#endif // LOADED_DLL_H

#define LOAD_ENGINE_DLL(variable, dll_path) ::EngineTest::LoadedDll variable(dll_path)

inline int& GetFailedCount() { static int count = 0; return count; }

// Is it true?
#define EXPECT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "FAILED: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
        } \
    } while (0)
// Is it false?
#define EXPECT_FALSE(expr) \
    do { \
        if (expr) { \
            std::cerr << "FAILED: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
        } \
    } while (0)
// Is it equal?
#define EXPECT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            std::cerr << "FAILED: " << #a << " != " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
        } \
    } while (0)
// Is it not equal?
#define EXPECT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            std::cerr << "FAILED: " << #a << " == " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
        } \
    } while (0)
// Is it true? If not, exit the program.
#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "FATAL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
            std::exit(1); \
        } \
    } while (0)
// Is it false? If not, exit the program.
#define ASSERT_FALSE(expr) \
    do { \
        if (expr) { \
            std::cerr << "FATAL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
            std::exit(1); \
        } \
    } while (0)
// Is it equal? If not, exit the program.
#define ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            std::cerr << "FATAL: " << #a << " != " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
            std::exit(1); \
        } \
    } while (0)
// Is it not equal? If not, exit the program.
#define ASSERT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            std::cerr << "FATAL: " << #a << " == " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
            std::exit(1); \
        } \
    } while (0)

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