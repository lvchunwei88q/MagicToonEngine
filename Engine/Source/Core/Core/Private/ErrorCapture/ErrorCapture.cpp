#include <ErrorCapture/ErrorCapture.h>
#include <Windows.h> // OutputDebugStringA

namespace Core {

    ErrorCapture::FuncType ErrorCapture::GetCaptureFunction;
    void ErrorCapture::RegisterCaptureFunction(FuncType func) {
        GetCaptureFunction = func;
    }

    void ErrorCapture::Capture(const std::string& errorMessage) {
        if (GetCaptureFunction) {
            GetCaptureFunction(errorMessage);
        }
        else {
#if _DEBUG
            OutputDebugStringA("Error CaptureFunction is nullptr! CaptureError:");
            OutputDebugStringA(errorMessage.c_str());
            OutputDebugStringA("\n");
#endif // _DEBUG
        }
    }
}