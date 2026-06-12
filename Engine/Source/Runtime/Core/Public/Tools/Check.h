#pragma once

#include <cassert>
#include <type_traits>

template<typename To, typename From>
inline To* SafeCast(From* ptr) {
    if (!ptr) return nullptr;
    
    // Check type relationships at compile time
    constexpr bool is_upcast = std::is_base_of_v<To, From>;
    constexpr bool is_downcast = std::is_base_of_v<From, To>;
    
    static_assert(is_upcast || is_downcast,
        "SafeCast: Types must be related by inheritance");
    
    if constexpr (is_upcast) {
        return static_cast<To*>(ptr);
    } 
    else if constexpr (is_downcast) {
#ifdef _DEBUG
        To* result = dynamic_cast<To*>(ptr);
        if (!result) {
            assert(result && "SafeCast: Type mismatch"); 
        }else{
            return result;
        }
#else
        return static_cast<To*>(ptr);
#endif
    }
    
    return nullptr;
}

// TODO 完善检测
#define DEFINE_MEMBER_CHECKER_BEGIN(CheckerName)      \
    template<typename T>                              \
    concept Has##CheckerName = 

#define DEFINE_MEMBER_CHECKER_ADD(Member)             \
    requires { &T::Member; } ||                       \

#define DEFINE_MEMBER_CHECKER_END(Member)             \
    requires { &T::Member; };                         \

#define CHECK_NO_MEMBER(ClassName, CheckerName) \
    static_assert(!Has##CheckerName<ClassName>, \
        "ERROR: " #ClassName " has conflicting member functions for " #CheckerName)