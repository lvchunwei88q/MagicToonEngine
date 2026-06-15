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

// // Check if it inherits from the parent class
#define FORBIDDEN_METHOD_CONCEPT(Space,method_name) \
    template<typename T> \
    concept HasForbiddenMethod_##Space_##method_name = requires(T t) { t.method_name(); }

#define COMBINE_FORBIDDEN_METHODS(Space,method_name) \
    HasForbiddenMethod_##Space_##method_name<T>

#define COMBINE_FORBIDDEN_NAME(Space) \
    Has##Space

#define CHECK_COMBINE_MEMBER(ClassName, CheckerName) \
    static_assert(Has##CheckerName<ClassName>, \
        "ERROR: " #ClassName " is missing required member functions! " \
        "Please ensure it inherits from " #CheckerName)