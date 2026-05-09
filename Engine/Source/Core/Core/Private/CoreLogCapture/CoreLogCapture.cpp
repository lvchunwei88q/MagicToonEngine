#include <CoreLogCapture/CoreLogCapture.h>
#include <Windows.h> // OutputDebugStringA

namespace Core {
#define TEMPPLATE_CAPTURE_IMPL(Target)                                                  \
    Target##Capture::FuncType Target##Capture::GetCaptureFunction;                      \
    void Target##Capture::RegisterCaptureFunction(FuncType func) {                      \
        GetCaptureFunction = func;                                                      \
    }                                                                                   \
    void Target##Capture::Capture(const std::string& Message) {                         \
        if (GetCaptureFunction) {                                                       \
            GetCaptureFunction(Message);                                                \
        }                                                                               \
        else {                                                                          \
            if constexpr (_DEBUG){                                                      \
                OutputDebugStringA("Error CaptureFunction is nullptr! CaptureError:");  \
                OutputDebugStringA(Message.c_str());                                    \
                OutputDebugStringA("\n");                                               \
            }                                                                           \
        }                                                                               \
    }

    TEMPPLATE_CAPTURE_IMPL(Error)
    TEMPPLATE_CAPTURE_IMPL(Warning)
    TEMPPLATE_CAPTURE_IMPL(Info)
}