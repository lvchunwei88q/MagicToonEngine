#include "Pipeline/GeneralPipeline.h"
#include <IO.h>
#include <exception>
#include <sstream>
#include <string>

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

		bool AppendReport(
			const std::wstring& path,
			const std::string& contents,
			const std::string& description)
		{
			try
			{
				if (IO::Exists(path))
				{
					IO::AppendText(path, contents);
				}
				else
				{
					if (!IO::CreateFile(path))
					{
						Utilitie::Log::Error("Could not create " + description);
						return false;
					}
					IO::WriteAllText(path, contents);
				}
			}
			catch (const std::exception& error)
			{
				Utilitie::Log::Error(
					"Could not append " + description + ": " +
					std::string(error.what()));
				return false;
			}
			return true;
		}
	}
    namespace GeneralPipeline {
        // print out the sorted classes and their members for debugging
        bool OutputMagicEngineClassInfo(){
            MGC ctx = MagicGlobalContext::Get();
            MagicGeneralPipelinedData& pipelineData = MagicGeneralPipelinedData::Get();
            const auto& classes = pipelineData.GetRefMagicClasses();

            std::ostringstream classDebugInfo;
			std::ostringstream classInfoReport;
            // Write the current time to the debug file
			const std::string writeTime = WriteTime();
            classDebugInfo << "[Info] " << writeTime << "\n";
			classInfoReport << "[Info] " << writeTime << "\n";
            
            for (const auto& classInfo : classes) {
    #if MAGIC_DEV
                std::cout << classInfo.className << std::endl;
    #endif
                classDebugInfo << classInfo.className << "\n";
                for (const auto& member : classInfo.members)
                {
                    if (!member.hasMMember) {
                        continue;
                    }
    #if MAGIC_DEV
                    std::cout << "   " << member.type << " - ";
                    std::cout << member.name << std::endl;
    #endif
                    classDebugInfo << "   " << member.type << " - ";
                    classDebugInfo << member.name << "\n";
                }

				classInfoReport << "Class: " << classInfo.qualifiedName << '\n'
					<< "Kind: " << classInfo.kind << '\n'
					<< "Header: " << classInfo.headerName << '\n'
					<< "Line: " << classInfo.lineNum + 1 << '\n'
					<< "MCLASS: yes";
				for (size_t i = 0; i < classInfo.mclassParams.size(); ++i)
				{
					classInfoReport << (i == 0 ? " (" : ", ")
						<< classInfo.mclassParams[i];
				}
				if (!classInfo.mclassParams.empty())
				{
					classInfoReport << ')';
				}
				classInfoReport << "\nGENERATE_BODY: yes";
				for (size_t i = 0; i < classInfo.generateBodyParams.size(); ++i)
				{
					classInfoReport << (i == 0 ? " (" : ", ")
						<< classInfo.generateBodyParams[i];
				}
				if (!classInfo.generateBodyParams.empty())
				{
					classInfoReport << ')';
				}
				classInfoReport << "\nBases:";
				for (const std::string& base : classInfo.baseClasses)
				{
					classInfoReport << ' ' << base;
				}
				classInfoReport << "\nFunctions:\n";
				for (const MagicClassFunction& function : classInfo.functions)
				{
					classInfoReport << "  [" << function.access << "] "
						<< function.signature << " (line "
						<< function.lineNum + 1 << ")\n";
				}
				classInfoReport << "MMEMBER variables:\n";
				for (const MemberVariable& member : classInfo.members)
				{
					if (!member.hasMMember)
					{
						continue;
					}
					classInfoReport << "  [" << member.access << "] "
						<< member.type << ' ' << member.name
						<< " (line " << member.lineNum << ") MMEMBER(";
					for (size_t i = 0; i < member.annotationParams.size(); ++i)
					{
						if (i != 0)
						{
							classInfoReport << ", ";
						}
						classInfoReport << member.annotationParams[i];
					}
					classInfoReport << ")\n";
				}
				classInfoReport << '\n';
            }

			const std::wstring classInfoReportPath =
				ctx.GetgenerateDir() + L"\\" MAGICENGINE_FILE_INFO_PATH_W
				L"\\" MAGICENGINE_CLASS_INFO_FILE_W;
            const std::wstring classDebugReportPath = 
                ctx.GetgenerateDir() + L"\\" MAGICENGINE_FILE_INFO_PATH_W 
                L"\\" MAGICENGINE_CLASS_METADATA_DEBUG_FILE_W;
            
			const bool classDebugReportWritten = AppendReport(
				classDebugReportPath, classDebugInfo.str(), "class metadata debug file");
			const bool classInfoReportWritten = AppendReport(
				classInfoReportPath,classInfoReport.str(),"class information report");

			return classDebugReportWritten && classInfoReportWritten;
        };
    }
}
