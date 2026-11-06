#include "Common.h"
#include <windows.h>
#include "Main.h"
#include <Editor.h>
#include <IO.h>

#if MAGICEDITOR_MODE
#include <shellapi.h>
#endif

#include "../Test/test.h" // 测试用文件

void TestAdd() {
    CLASS_B* b = ObjectFactory::Create<CLASS_B>();
    const std::wstring objectBPath = L"D:\\Test\\MTE\\TEST\\ObjectB.masset";
    if (IO::Exists(objectBPath)) {
        std::vector<uint8_t> objectFileData = IO::ReadAllUINT8Bytes(objectBPath);
        BinaryWriter objectData;
        objectData.Write(objectFileData.data(), objectFileData.size());
        b->Deserialize(objectData);
    }
    b->x++;
    IO::CreateDirectory(L"D:\\Test\\MTE\\TEST");
    const BinaryReader objectData = b->Serialize();
    IO::WriteAllBytes(objectBPath, objectData.DataU8(), objectData.Size());
    delete b;

    std::shared_ptr<CLASS_X> X = ObjectFactory::CreateShared<CLASS_X>();
    const std::wstring objectXPath = L"D:\\Test\\MTE\\TEST\\ObjectTest.masset";
    if (IO::Exists(objectXPath)) {
        std::vector<uint8_t> objectFileData = IO::ReadAllUINT8Bytes(objectXPath);
        BinaryWriter objectData;
        objectData.Write(objectFileData.data(), objectFileData.size());
        X->Deserialize(objectData);
    }
    X->o = X->o + 100.0f;
    X->Testint3 = X->Testint3 + int3(0,1,0);
    IO::CreateDirectory(L"D:\\Test\\MTE\\TEST");
    const BinaryReader objectData1 = X->Serialize();
    IO::WriteAllBytes(objectXPath, objectData1.DataU8(), objectData1.Size());

    LuaVMTest::RunAllTests();
}

#if MAGICEDITOR_MODE
static int argc = 0;
static LPWSTR* argv;

// 初始化命令行参数
bool InitCommandLine() {
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    return true;
}

void CleanupCommandLine() {
    LocalFree(argv);
}

// GUI 入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
    if (!InitCommandLine()) {
        return 1;
    }

    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;
    std::cout << "Engine Init (GUI Mode) ... " << std::endl;

#else // MAGICEDITOR_CMD
int wmain(int argc, wchar_t* argv[]) {
    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;
    std::cout << "Engine Init (CMD Mode) ... " << std::endl;
#endif

    std::vector<std::wstring> argsW(argv, argv + argc);
    if (!CommandCheck(argsW)) 
    { 
        std::cout << "The engine has not yet opened a project!" << std::endl;
        RunMagicLauncher();
        return 0;
    }

    RegisterSubsystemCallback();

    SubSystemError error = Core::SubsystemControl::Init();
    if (!error.error) {
        //每个进程有独立的虚拟地址空间，泄漏只影响自己的进程，不会污染其他程序或系统 所以不需要释放了
        std::cout << "There was a problem during engine initialization!" << std::endl;
        std::cout << "Problem Subsystem:" << error.target << std::endl;
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        std::cin.get();
        return 0;
    }

    std::cout << "Engine initialized successfully. " << std::endl;

    TestAdd();

    FreeConsole(); // Free Console
    Editor::Editor::Get().SetEngineState(Editor::Editor::EngineState::Run);
	Editor::Editor::Get().Run(); // Run Engine Editor

	Core::SubsystemControl::Uninstall();

#if MAGICEDITOR_MODE
    CleanupCommandLine();
#endif

	return 0;
}