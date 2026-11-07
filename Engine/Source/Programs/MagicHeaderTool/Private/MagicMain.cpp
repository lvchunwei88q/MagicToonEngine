#include <cmdline/cmdline.h> // Parameter Parsing Library
#include <Subsystem/Subsystem.h>
#include <Jobsystem/JobSystem.h> // JobSystem
#include <IO.h> // File IO
#include "MagicHeaderTool.h"

#include "MagicUtilities.h"

using namespace MHT;

int main(int argc, char* argv[]) {
    cmdline::parser parser;

    // Define command-line arguments
    parser.add<std::string>("generate-dir", 'g', "File directory for generating code", true, "");
    parser.add("help", '?', "Print help");

    // Parse the command-line arguments
    if (!parser.parse_check(argc, argv)) {
        return 0;
    }

    // Get the generate directory from the parsed arguments
    std::string generateDir = parser.get<std::string>("generate-dir");
	std::wstring generateDirW = IO::ToWideString(generateDir);
    std::wstring generateInfoFilePath = generateDirW + L"\\" MAGICENGINE_FILE_INFO_PATH_W L"\\" MAGICENGINE_HEADER_INFO_FILE_W;

    // Set the generate directory and info file path in the global context
    MGC ctx = MagicGlobalContext::Get();
    ctx.SetgenerateDir(generateDirW);
    ctx.SetgenerateInfoFilePath(generateInfoFilePath);

    // Initialize the Core Subsystem
    Core::SubsystemControl::Init();
    {
        MagicHeaderTool MagicHT{};
        // read the generate info file and run the code generation process  
        if (MagicHT.readGenerateInfoFile()) {
            Utilitie::Log::Info("Start reading all engine header file contents");
            if (MagicHT.Run()) {
                Utilitie::Log::Info("Code generation completed successfully");
                Core::JobSystem::Get().Uninstall();
                return 0; // success
            }
        }
    }
    // Uninstall the Core Subsystem and JobSystem
    Core::SubsystemControl::Uninstall();
    return 1; // error
}