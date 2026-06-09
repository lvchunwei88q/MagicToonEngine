#include "Tools/cmdline.h" // 参数解析库
#include "MagicHeaderTool.h"

#include "Tools/Tool.h"
#include "Tools/JobSystem.h"

using namespace MHT;

int main(int argc, char* argv[]) {
    cmdline::parser parser;
    JobSystem::Get().Init();

    parser.add<std::string>("generate-dir", 'g', "File directory for generating code", true, "");
    parser.add("help", '?', "Print help");

    parser.parse_check(argc, argv);

    std::string generateDir = parser.get<std::string>("generate-dir");

	std::wstring generateDirW = TOOL::ToWideString(generateDir);
    generateDirW += L"\\engine_info\\engine_headers.buildmeta";

    MagicHeaderTool MagicBT;
    if (MagicBT.readGenerateInfoFile(generateDirW)) {
        TOOL::Log::Info("Start reading all engine header file contents");
        if (MagicBT.readHeaderFiles()) {
            TOOL::Log::Info("Start building code");
            if (MagicBT.RunBuildPipeline()) {
                TOOL::Log::Info("Code generation completed successfully");
                JobSystem::Get().Uninstall();
                return 0; // success
            }
        }
    }

    JobSystem::Get().Uninstall();
    return 1; // error
}