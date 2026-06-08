#include "Tools/cmdline.h" // 参数解析库
#include "MagicBuildTool.h"

#include <IO.h>

using namespace MBT;

int main(int argc, char* argv[]) {
    cmdline::parser parser;

    parser.add<std::string>("generate-dir", 'g', "File directory for generating code", true, "");
    parser.add("help", '?', "Print help");

    parser.parse_check(argc, argv);

    std::string generateDir = parser.get<std::string>("generate-dir");

	std::wstring generateDirW = IO::ToWideString(generateDir);
    generateDirW += L"\\engine_info\\engine_headers.buildmeta";

	MagicBuildTool MagicBT;
    if (MagicBT.readGenerateInfoFile(generateDirW)) {
        Log::Info("Start reading all engine header file contents");
        if (MagicBT.readHeaderFiles()) {
            Log::Info("Start building code");
            if (MagicBT.RunBuildPipeline()) {
				Log::Info("Code generation completed successfully");
                return 0; // success
            }
        }
    }

    return 1; // error
}