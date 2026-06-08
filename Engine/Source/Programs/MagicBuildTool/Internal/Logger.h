// Logger.h
#pragma once

#include <iostream>

class Log {
public:
    static void Info(const std::string& msg) {
#ifdef _DEBUG
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