#include "Pipeline/GeneralPipeline.h"
#include <IO.h>

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
            const auto& order_magicEngineClasss = pipelineData.GetRefMagicEngineClasss();

            std::wstring ClassDebugFilePath = ctx.GetgenerateDir() + L"\\" MAGICENGINE_FILE_INFO_PATH_W L"\\" MAGICENGINE_CLASS_METADATA_DEBUG_FILE_W;
            std::ostringstream oss;
            // Write the current time to the debug file
            oss << "[Info] " << WriteTime() << "\n";
            for (size_t a = 0; a < order_magicEngineClasss.size(); a++) {
                auto& OrderClass = order_magicEngineClasss[a];
    #if MAGIC_DEV
                std::cout << OrderClass.className << std::endl;
    #endif
                oss << OrderClass.className << "\n";
                for (size_t i = 0; i < OrderClass.members.size(); i++)
                {
                    auto& members = OrderClass.members[i];
    #if MAGIC_DEV
                    std::cout << "   " << members.type << " - ";
                    std::cout << members.name << std::endl;
    #endif
                    oss << "   " << members.type << " - ";
                    oss << members.name << "\n";
                }
            }

            // Write to or add to a file
            std::string debugText = oss.str();
            if(IO::Exists(ClassDebugFilePath)){
                IO::AppendText(ClassDebugFilePath, debugText);
            }
            else{
                IO::CreateFile(ClassDebugFilePath);
                IO::WriteAllText(ClassDebugFilePath, debugText);
            }

            return true;
        };
    }
}
