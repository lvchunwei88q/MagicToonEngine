#pragma once
#include <Subsystem/Subsystem.h>
#include <Common/Singleton.h>

template<typename T>
class SubsystemTemplate : public Core::Subsystem, public Singleton<T>
{
public:
    static void RegisterStatic() {
        const char* name = typeid(T).name();
        SubsystemTemplate::Get().template Register<T>();
    }
};

#define CAT(a, b) a##b
#define AUTO_REGISTER(T) \
    static bool CAT(T, _registered) = []() { \
        T::RegisterStatic(); \
        return true; \
    }(); \
    bool CAT(T, _dummy) = CAT(T, _registered)