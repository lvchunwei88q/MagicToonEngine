#pragma once
#include <Core.h>
#include <Subsystem/Subsystem.h>
#include <iostream>
#include <vector>

enum class OperationType {
    NOT,
    SpecifyProJect
};

bool CommandCheck(const std::vector<std::wstring>& argsW);
void RunMagicLauncher();
void RegisterSubsystemCallback();