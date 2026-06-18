#include <cmdline/cmdline.h> // 参数解析库
#include "MagicHeaderTool.h"

#include "Tools/Tool.h"
#include "Tools/JobSystem.h"

using namespace MHT;

int main(int argc, char* argv[]) {
    cmdline::parser parser;

    parser.add<std::string>("generate-dir", 'g', "File directory for generating code", true, "");
    parser.add("help", '?', "Print help");

    parser.parse_check(argc, argv);

    std::string generateDir = parser.get<std::string>("generate-dir");

	std::wstring generateDirW = TOOL::ToWideString(generateDir);
    std::wstring generateFilePath = generateDirW + L"\\" MAGICENGINE_FILE_INFO_PATH "\\engine_headers.buildmeta";

    JobSystem::Get().Init();

    MagicHeaderTool MagicHT;
    if (MagicHT.readGenerateInfoFile(generateFilePath, generateDirW)) {
        TOOL::Log::Info("Start reading all engine header file contents");
        if (MagicHT.Run()) {
            TOOL::Log::Info("Code generation completed successfully");
            JobSystem::Get().Uninstall();
            return 0; // success
        }
    }

    JobSystem::Get().Uninstall();
    return 1; // error
}