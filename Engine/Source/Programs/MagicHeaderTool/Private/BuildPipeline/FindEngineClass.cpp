#include "BuildPipeline.h"
#include <vector>
#include <ctre/ctre.hpp>

namespace MHT {
    namespace {
        void SplitParams(std::string_view args, std::vector<std::string>& outParams) {
            std::string_view remaining = args;
            while (!remaining.empty()) {
                size_t comma_pos = remaining.find(',');
                std::string_view param = (comma_pos == std::string_view::npos)
                    ? remaining
                    : remaining.substr(0, comma_pos);

                // 去除前导空白
                size_t start = param.find_first_not_of(" \t");
                if (start == std::string_view::npos) {
                    remaining = (comma_pos == std::string_view::npos)
                        ? std::string_view()
                        : remaining.substr(comma_pos + 1);
                    continue;
                }
                // 去除尾部空白
                size_t end = param.find_last_not_of(" \t");
                param = param.substr(start, end - start + 1);

                if (!param.empty()) {
                    std::string paramString = std::string(param);
                    outParams.push_back(paramString);
                }

                // 移动到下一个参数
                remaining = (comma_pos == std::string_view::npos)
                    ? std::string_view()
                    : remaining.substr(comma_pos + 1);
            }
        }

        // 检测 MCLASS(...); 并提取括号内的参数
        bool ParseMClass(const std::string& line, std::vector<std::string>& outParams) {

            // 匹配 MCLASS( ... );
            if (auto match = ctre::match<R"(^\s*MCLASS\s*\(\s*([^)]*?)\s*\)\s*;?\s*$)">(line)) {
                std::string_view args = match.get<1>();
                SplitParams(args, outParams);
                return true;
            }
            return false;
        }

        // 检测当前行是否是 class 签名
        bool IsClassSignature(const std::string& line) {
            // 匹配 class 关键字（作为独立单词），后面可以跟各种内容
            return ctre::search<R"(^\s*class\s)">(line) ||  // class 后有空白
                ctre::search<R"(^\s*class\b)">(line);    // class 作为完整单词
        }

        // 检测固定字符串 GENERATE_BODY();
        bool ParseGenerateBody(const std::string& line, std::vector<std::string>& outParams) {

            // 匹配 GENERATE_BODY( ... );
            if (auto match = ctre::match<R"(^\s*GENERATE_BODY\s*\(\s*([^)]*?)\s*\)\s*;?\s*$)">(line)) {
                std::string_view args = match.get<1>();
                SplitParams(args, outParams);
                return true;
            }
            return false;
        }


        // 获取到类名 class a or class XXX_API a
        std::string ExtractClassName(const std::string& line) {
            if (auto search = ctre::search<R"(^\s*class\s+(?:\w+_API\s+)?(\w+))">(line)) {
                return search.get<1>().to_string();
            }
            return "";
        }
    }

    namespace Pipeline {
        struct LocalMagicClassParamsInfo {
            std::vector<std::string> mclassParams = {};
            std::vector<std::string> mgeneratebodyParams = {};
            std::string currentClassName;       // 当前正在处理的类名
            size_t currentClassLineNum = 0;     // 当前正在处理的类所在行数

            void clear() {
                currentClassName.clear();
                mclassParams.clear();
                mgeneratebodyParams.clear();
            }
        };

        bool FindEngineClass()
        {
            std::vector<MagicEngineHeader> magicEngineHeaders = MagicBuildData::Get().GetMagicEngineHeaders();

            std::vector<MagicEngineClass> magicEngineClasss;
            for (size_t i = 0; i < magicEngineHeaders.size(); i++)
            {
                auto& EngineHeader = magicEngineHeaders[i];

                bool NeedtoFindGenerator = false; // 默认不需要找
                LocalMagicClassParamsInfo Info;

                for (size_t y = 0; y < EngineHeader.lines.size(); y++)
                {
                    std::string& line = EngineHeader.lines[y];

                    if (ParseMClass(line, Info.mclassParams)) {
                        if (Info.mclassParams.size() <= 0) {
                            TOOL::Log::Error("Cannot determine the generation type without filling in the corresponding MCLASS parameter:"
                                                                        + EngineHeader.headerName + " Line:" + std::to_string(y + 1));
                            return false;
                        }

                        if (NeedtoFindGenerator) { // 此时是在寻找ClassBody
                            TOOL::Log::Error("No body corresponding to the Class was found:" + EngineHeader.headerName + " Line:" + std::to_string(y + 1));
                            return false;
                        }
                        
                        if (y + 1 >= EngineHeader.lines.size()) {
                            TOOL::Log::Error("MCLASS at end of file without class: " + EngineHeader.headerName);
                            return false;
                        }
                        std::string& next_line = EngineHeader.lines[y + 1];
                        if (!IsClassSignature(next_line)) {
                            TOOL::Log::Error("No CLASS corresponding to MCLASS was found:" + EngineHeader.headerName + " Line:" + std::to_string(y + 1));
                            return false;
                        }

                        Info.currentClassName = ExtractClassName(next_line);
                        Info.currentClassLineNum = y; // 记录类宏定义所在行数

                        NeedtoFindGenerator = true; // 找到 MCLASS 了，接下来需要找 GENERATE_BODY
                    }

                    if (NeedtoFindGenerator) {
                        if (ParseGenerateBody(line, Info.mgeneratebodyParams)) {
                            if (Info.mgeneratebodyParams.size() <= 0) {
                                TOOL::Log::Error("If the corresponding GenerateBody parameter is not filled in, the type cannot be generated.:"
                                    + EngineHeader.headerName + " Line:" + std::to_string(y + 1));
                                return false;
                            }
                            NeedtoFindGenerator = false; // 找到 GENERATE_BODY 了，重置状态


                            magicEngineClasss.push_back({ std::move(Info.mclassParams),std::move(Info.mgeneratebodyParams),
                                                            EngineHeader.headerName,EngineHeader.moudelName,
                                                            Info.currentClassName,Info.currentClassLineNum });
                            Info.clear();
                        }
                    }
                }
                if (NeedtoFindGenerator) { // 如果找到了 MCLASS 但是没有找到 GENERATE_BODY 就报错
                    TOOL::Log::Error("No GENERATE_BODY corresponding to MCLASS was found:" + EngineHeader.headerName);
                    return false;
                }
            }

            if (magicEngineClasss.size() <= 0)SetGenerateCompleted();
			MagicBuildData::Get().SetMagicEngineClasss(magicEngineClasss);

            return true;
        }
    }
}