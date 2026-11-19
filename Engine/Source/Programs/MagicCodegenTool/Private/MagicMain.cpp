#include <cmdline/cmdline.h> // Parameter Parsing Library
#include <Subsystem/Subsystem.h>
#include <Jobsystem/JobSystem.h> // JobSystem
#include <IO.h> // File IO
#include "MagicCodegenTool.h"
#include "Pipeline/GeneralPipeline.h"

#include "MagicUtilities.h"

using namespace MCT;

int main(int argc, char* argv[]) {
    // Set initial line break
    std::cout << "\n";
    // Initialize the cmdline parser
    cmdline::parser parser;

    // Define command-line arguments
    parser.add<std::string>("generate-dir", 'g', "File directory for generating code", true, "");
    parser.add("help", '?', "Print help");

    // Parse the command-line arguments.
    // Exit codes: 0 = success (including an explicit --help / -?),
    //             2 = bad arguments, 1 = code generation failed.
    // If you return 0 every time parsing fails, then CMake's
    // add_custom_target(${MAGIC_CODEGEN_TOOL_RUN} ALL ...) reported the codegen
    // step as successful and the build only failed later with a wall of
    // "cannot open xxx.object.generate.h".
    if (!parser.parse_check(argc, argv)) {
        // parse_check() already printed the error message and usage to stderr;
        // --help / -? is a normal request, everything else is a real failure.
        return parser.exist("help") ? 0 : 2;
    }

    // Get the generate directory from the parsed arguments
    const IO::path generateDir =
        std::filesystem::u8path(parser.get<std::string>("generate-dir"));
    const IO::path generateInfoFilePath =
        generateDir / MAGICENGINE_FILE_INFO_PATH / MAGICENGINE_HEADER_INFO_FILE;

    // Set the generate directory and info file path in the global context
    MGC ctx = MagicGlobalContext::Get();
    ctx.SetgenerateDir(generateDir);
    ctx.SetgenerateInfoFilePath(generateInfoFilePath);

    // Initialize the Core Subsystem
    Core::SubsystemControl::Init();
    {
        MagicCodegenTool MagicCT{};
        // read the generate info file and run the code generation process  
        if (MagicCT.readGenerateInfoFile()) {
            Utilitie::Log::Info("Start reading all engine header file contents");
            if (MagicCT.Run()) {
                Utilitie::Log::Info("Code generation completed successfully");
                // Uninstall the Core Subsystem and JobSystem
                Core::SubsystemControl::Uninstall();
                return 0; // success
            }
        }
    }
    // Uninstall the Core Subsystem and JobSystem
    Core::SubsystemControl::Uninstall();
    return 1; // error
}
