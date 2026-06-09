// Logger.h
#pragma once

#include <iostream>
#define DETAILED 1

class Log {
public:
    static void Info(const std::string& msg) {
#if defined(_DEBUG) || defined(DETAILED)
        std::cout << "[MagicBuildTool][INFO] " << msg << "\n";
#endif
    }

    static void Warning(const std::string& msg) {
        std::cout << "[MagicBuildTool][WARNING] " << msg << "\n";
    }

    static void Error(const std::string& msg) {
        std::cerr << "[MagicBuildTool][ERROR] " << msg << "\n";
    }
};