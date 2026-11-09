#include "Pipeline/GeneralPipeline.h"
#include <vector>
#include <ctre/ctre.hpp>

namespace MHT {
    namespace {
        struct LocalMagicClassParamsInfo {
            std::vector<std::string> mclassParams = {};
            std::vector<std::string> mgeneratebodyParams = {};
            std::string currentClassName;       // The class name currently being processed
            size_t currentClassLineNum = 0;     // The line number of the class currently being processed

            void clear() {
                currentClassName.clear();
                mclassParams.clear();
                mgeneratebodyParams.clear();
            }
        };

        void SplitParams(std::string_view args, std::vector<std::string>& outParams) {
            std::string_view remaining = args;
            while (!remaining.empty()) {
                size_t comma_pos = remaining.find(',');
                std::string_view param = (comma_pos == std::string_view::npos)
                    ? remaining
                    : remaining.substr(0, comma_pos);

                // Remove leading spaces
                size_t start = param.find_first_not_of(" \t");
                if (start == std::string_view::npos) {
                    remaining = (comma_pos == std::string_view::npos)
                        ? std::string_view()
                        : remaining.substr(comma_pos + 1);
                    continue;
                }
                // Remove trailing spaces
                size_t end = param.find_last_not_of(" \t");
                param = param.substr(start, end - start + 1);

                if (!param.empty()) {
                    std::string paramString = std::string(param);
                    outParams.push_back(paramString);
                }

                // Move to the next parameter
                remaining = (comma_pos == std::string_view::npos)
                    ? std::string_view()
                    : remaining.substr(comma_pos + 1);
            }
        }

        // Check MCLASS(...); and extract parameters within parentheses
        bool ParseMClass(const std::string& line, std::vector<std::string>& outParams) {

            // Match MCLASS( ... );
            if (auto match = ctre::match<R"(^\s*MCLASS\s*\(\s*([^)]*?)\s*\)\s*;?\s*$)">(line)) {
                std::string_view args = match.get<1>();
                SplitParams(args, outParams);
                return true;
            }
            return false;
        }

        // Check if the current line is a class signature
        bool IsClassSignature(const std::string& line) {
            // The 'match class' keyword (as a standalone word) can be followed by all kinds of stuff
            return ctre::search<R"(^\s*class\s)">(line) ||  // There's a space after class
                ctre::search<R"(^\s*class\b)">(line);    // class as a complete word
        }

        // Check for the fixed string GENERATE_BODY();
        bool ParseGenerateBody(const std::string& line, std::vector<std::string>& outParams) {

            // Match GENERATE_BODY( ... );
            if (auto match = ctre::match<R"(^\s*GENERATE_BODY\s*\(\s*([^)]*?)\s*\)\s*;?\s*$)">(line)) {
                std::string_view args = match.get<1>();
                SplitParams(args, outParams);
                return true;
            }
            return false;
        }


        // Get the class name class a or class XXX_API a
        std::string ExtractClassName(const std::string& line) {
            if (auto search = ctre::search<R"(^\s*class\s+(?:\w+_API\s+)?(\w+))">(line)) {
                return search.get<1>().to_string();
            }
            return "";
        }
    }

    namespace GeneralPipeline {
        bool FindEngineClass()
        {
            // Get Magic Global Context
            MGC ctx = MagicGlobalContext::Get();
            std::vector<MagicEngineHeader> magicEngineHeaders = MagicGeneralPipelinedData::Get().GetMagicEngineHeaders();

            std::vector<MagicEngineClass> magicEngineClasss;
            for (size_t i = 0; i < magicEngineHeaders.size(); i++)
            {
                auto& EngineHeader = magicEngineHeaders[i];

                // Mark whether you need to look for GENERATE_BODY By default, no need to look
                bool IsNeedtoFindGenerator = false;
                LocalMagicClassParamsInfo Info;

                for (size_t y = 0; y < EngineHeader.lines.size(); y++)
                {
                    std::string& line = EngineHeader.lines[y];

                    if (ParseMClass(line, Info.mclassParams)) {
                        // So abilities should be optional and you shouldn't be forced to use just one.
                        // if (Info.mclassParams.size() <= 0) {
                        //     Utilitie::Log::Error("Cannot determine the generation type without filling in the corresponding MCLASS parameter:"
                        //                                                 + EngineHeader.headerName + " Line:" + std::to_string(y + 1));
                        //     return false;
                        // }

                        // If at this point you are already looking for GENERATE_BODY (after finding MCLASS), but then find another one, an exception should be thrown.
                        if (IsNeedtoFindGenerator) {
                            Utilitie::Log::Error("No body corresponding to the Class was found:" + EngineHeader.headerName + " Line:" + std::to_string(y + 1));
                            return false;
                        }
                        
                        // If MCLASS is found but the next line is the end of the file, throw an exception
                        if (y + 1 >= EngineHeader.lines.size()) {
                            Utilitie::Log::Error("MCLASS at end of file without class: " + EngineHeader.headerName);
                            return false;
                        }

                        // If the next line is not a class signature, an exception should be thrown.
                        std::string& next_line = EngineHeader.lines[y + 1];
                        if (!IsClassSignature(next_line)) {
                            Utilitie::Log::Error("No CLASS corresponding to MCLASS was found:" + EngineHeader.headerName + " Line:" + std::to_string(y + 1));
                            return false;
                        }

                        Info.currentClassName = ExtractClassName(next_line);
                        Info.currentClassLineNum = y; // The line number where the macro definition is recorded

                        //After finding MCLASS, you need to look for GENERATE_BODY
                        IsNeedtoFindGenerator = true;
                    }

                    if (IsNeedtoFindGenerator) {
                        if (ParseGenerateBody(line, Info.mgeneratebodyParams)) {
                            if (Info.mgeneratebodyParams.size() <= 0) {
                                Utilitie::Log::Error("If the corresponding GenerateBody parameter is not filled in, the type cannot be generated.:"
                                    + EngineHeader.headerName + " Line:" + std::to_string(y + 1));
                                return false;
                            }

                            // Found GENERATE_BODY, resetting the state to avoid affecting subsequent judgments.
                            IsNeedtoFindGenerator = false;

                            // add the class to the list of classes
                            magicEngineClasss.push_back({ std::move(Info.mclassParams),std::move(Info.mgeneratebodyParams),
                                                            EngineHeader.headerName,EngineHeader.moudelName,
                                                            Info.currentClassName,Info.currentClassLineNum });
                            Info.clear();
                        }
                    }
                }
                if (IsNeedtoFindGenerator) { // It throws an error if MCLASS is found but GENERATE_BODY isn't.
                    Utilitie::Log::Error("No GENERATE_BODY corresponding to MCLASS was found:" + EngineHeader.headerName);
                    return false;
                }
            }

            // If the number of Classes found is less than or equal to 0, then it means there's no need to continue processing.
            if (magicEngineClasss.size() <= 0) ctx.SetGenerateCompleted();

            // Finally, save the found classes to MagicBuildData
			MagicGeneralPipelinedData::Get().SetMagicEngineClasss(magicEngineClasss);

            return true;
        }
    }
}
