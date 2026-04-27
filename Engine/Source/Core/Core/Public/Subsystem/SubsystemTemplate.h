#pragma once
#include <Subsystem/Subsystem.h>
#include <Common/Singleton.h>

// the subsystems template Cannot be used as an export API!!!
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

/*
* 在使用模板时这个Class将不可以用于导出API。
* 如果需要导出API，请使用AUTO_REGISTER_SINGLETON宏，并在对应的cpp文件中实现注册逻辑。
* 并且直接继承Subsystem和Singleton，而不是SubsystemTemplate。
*/
#define AUTO_REGISTER_SINGLETON(T, Priority_)                               \
struct CAT(T, _AutoRegister)                                                 \
{                                                                           \
    CAT(T, _AutoRegister)() {                                                \
        T::Get().Register<T, Core::SubsystemContext::Priority::Priority_>();\
    }                                                                       \
};                                                                          \
static CAT(T, _AutoRegister) CAT(s_##T, _AutoRegister);

#define AUTO_REGISTER_SINGLETON_INCLUDE(T)                        \
struct CAT(T, _AutoRegister);                                     \
extern CAT(T, _AutoRegister) CAT(s_##T, _AutoRegister);