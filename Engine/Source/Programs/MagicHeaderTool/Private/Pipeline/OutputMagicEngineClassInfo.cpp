#include "Pipeline/GeneralPipeline.h"
#include <IO.h>
#include <exception>

namespace MHT 
{
	namespace {
		// Format: Year-Month-Day-Hour:Minute:Second (no leading zeros for month and day, pad zeros for hour, minute, and second)
		std::string WriteTime() {
			std::time_t t = std::time(nullptr);
			std::tm* tm = std::localtime(&t);

			auto two = [](int v) { return (v < 10 ? "0" : "") + std::to_string(v); };

			return "Write time: " +
				std::to_string(tm->tm_year + 1900) + "-" +
				std::to_string(tm->tm_mon + 1) + "-" +
				std::to_string(tm->tm_mday) + "-" +
				two(tm->tm_hour) + ":" +
				two(tm->tm_min) + ":" +
				two(tm->tm_sec);
		}
	}
    namespace GeneralPipeline {
        // print out the sorted classes and their members for debugging
        bool OutputMagicEngineClassInfo(){
            MGC ctx = MagicGlobalContext::Get();
            MagicGeneralPipelinedData& pipelineData = MagicGeneralPipelinedData::Get();
            const auto& classes = pipelineData.GetRefMagicClasses();

            std::wstring ClassDebugFilePath = ctx.GetgenerateDir() + L"\\" MAGICENGINE_FILE_INFO_PATH_W L"\\" MAGICENGINE_CLASS_METADATA_DEBUG_FILE_W;
            std::ostringstream oss;
            // Write the current time to the debug file
            oss << "[Info] " << WriteTime() << "\n";
            for (const auto& classInfo : classes) {
    #if MAGIC_DEV
                std::cout << classInfo.className << std::endl;
    #endif
                oss << classInfo.className << "\n";
                for (const auto& member : classInfo.members)
                {
                    if (!member.hasMMember) {
                        continue;
                    }
    #if MAGIC_DEV
                    std::cout << "   " << member.type << " - ";
                    std::cout << member.name << std::endl;
    #endif
                    oss << "   " << member.type << " - ";
                    oss << member.name << "\n";
                }
            }

            // Write to or add to a file
            std::string debugText = oss.str();
            try {
                if(IO::Exists(ClassDebugFilePath)){
                    IO::AppendText(ClassDebugFilePath, debugText);
                }
                else{
                    if (!IO::CreateFile(ClassDebugFilePath)) {
                        Utilitie::Log::Error("Could not create class metadata debug file");
                        return false;
                    }
                    IO::WriteAllText(ClassDebugFilePath, debugText);
                }
            }
            catch (const std::exception& error) {
                Utilitie::Log::Error(
                    "Could not write class metadata debug file: " +
                    std::string(error.what()));
                return false;
            }

            return true;
        };
    }
}
