#include <cmdline/cmdline.h> // Parameter Parsing Library
#include <Jobsystem/JobSystem.h> // JobSystem
#include <IO.h> // File IO
#include "MagicHeaderTool.h"

#include "Tools/Tool.h"

using namespace MHT;

int main(int argc, char* argv[]) {
    cmdline::parser parser;

    parser.add<std::string>("generate-dir", 'g', "File directory for generating code", true, "");
    parser.add("help", '?', "Print help");

    if (!parser.parse_check(argc, argv)) {
        return 0;
    }

    std::string generateDir = parser.get<std::string>("generate-dir");

	std::wstring generateDirW = IO::ToWideString(generateDir);
    std::wstring generateFilePath = generateDirW + L"\\" MAGICENGINE_FILE_INFO_PATH "\\engine_headers.buildmeta";

    Core::JobSystem::Get().Init();

    MagicHeaderTool MagicHT;
    if (MagicHT.readGenerateInfoFile(generateFilePath, generateDirW)) {
        TOOL::Log::Info("Start reading all engine header file contents");
        if (MagicHT.Run()) {
            TOOL::Log::Info("Code generation completed successfully");
            Core::JobSystem::Get().Uninstall();
            return 0; // success
        }
    }

    Core::JobSystem::Get().Uninstall();
    return 1; // error
}