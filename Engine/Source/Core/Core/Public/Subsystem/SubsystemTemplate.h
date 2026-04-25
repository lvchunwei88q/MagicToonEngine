#pragma once
#include <Subsystem/Subsystem.h>
#include <Common/Singleton.h>

template<typename T, Core::SubsystemContext::Priority Priority>
class SubsystemTemplate : public Core::Subsystem, public Singleton<T>
{
public:
    static void RegisterStatic() {
        SubsystemTemplate::Get().template Register<T,Priority>();
    }
};

#define CAT(a, b) a##b
#define AUTO_REGISTER(T) \
    inline static bool CAT(T, _registered) = []() { \
        T::RegisterStatic(); \
        return true; \
    }(); \
    inline bool CAT(T, _dummy) = CAT(T, _registered);