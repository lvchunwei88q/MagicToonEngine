#include "BuildPipeline.h"
#include <vector>
#include <regex>

namespace MHT {
    namespace {
        // 检测 MCLASS(...); 并提取括号内的参数
        bool ParseMClass(const std::string& line, std::vector<std::string>& outParams) {
            outParams.clear();
            size_t pos = 0;
            // 跳过前导空格
            while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
                ++pos;
            }
            // 检查是否以 MCLASS 开头
            if (line.size() - pos < 6 || line[pos] != 'M' || line[pos + 1] != 'C' ||
                line[pos + 2] != 'L' || line[pos + 3] != 'A' || line[pos + 4] != 'S' || line[pos + 5] != 'S') {
                return false;
            }

            // 匹配 MCLASS( ... ); 允许前后空格，括号内任意字符
            std::regex pattern(R"(^\s*MCLASS\s*\(\s*([^)]*?)\s*\)\s*;?\s*$)");
            std::smatch match;
            if (std::regex_search(line, match, pattern) && match.size() >= 2) {
                std::string args = match[1].str();
                // 按逗号分割
                std::regex comma(R"(\s*,\s*)");
                std::sregex_token_iterator it(args.begin(), args.end(), comma, -1);
                std::sregex_token_iterator end;
                for (; it != end; ++it) {
                    std::string param = it->str();
                    // 去除首尾空白
                    param = std::regex_replace(param, std::regex(R"(^\s+|\s+$)"), "");
                    if (!param.empty()) {
                        outParams.push_back(param);
                    }
                }
                return true;
            }
            return false;
        }

        // 检测当前行是否是 class 签名
        bool IsClassSignature(const std::string& line) {
            // 匹配: class 关键字，后面有空白
            std::regex pattern(R"(^\s*class\s+)");
            return std::regex_search(line, pattern);
        }

        // 检测固定字符串 GENERATE_BODY();
        bool HasGenerateBody(const std::string& line) {
            std::regex pattern(R"(^\s*GENERATE_BODY\s*\(\s*\)\s*;?\s*$)");
            return std::regex_search(line, pattern);
        }

		// 获取到类名 class a or class XXX_API a
        std::string ExtractClassName(const std::string& line) {
            std::regex pattern(R"(^\s*class\s+(?:\w+_API\s+)?(\w+))");
            std::smatch match;
            if (std::regex_search(line, match, pattern) && match.size() > 1) {
                return match[1].str();
            }
            return "";
        }
    }

    namespace Pipeline {
        bool FindEngineClass()
        {
            std::vector<MagicEngineHeader> magicEngineHeaders = MagicBuildData::Get().GetMagicEngineHeaders();

            std::vector<MagicEngineClass> magicEngineClasss;
            for (size_t i = 0; i < magicEngineHeaders.size(); i++)
            {
                auto& EngineHeader = magicEngineHeaders[i];

                bool NeedtoFindGenerator = false; // 默认不需要找
				std::string currentClassName; // 当前正在处理的类名
				size_t currentClassLineNum = 0; // 当前正在处理的类所在行数
                for (size_t i = 0; i < EngineHeader.lines.size(); i++)
                {
                    std::string& line = EngineHeader.lines[i];

                    std::vector<std::string> mclassParams;
                    if (ParseMClass(line, mclassParams)) {
                        if (mclassParams.size() > 0) {
                            TOOL::Log::Error("Position class definition parameters:" + EngineHeader.headerName + " Line:" + std::to_string(i + 1));
                            return false;
                        }

                        std::string& next_line = EngineHeader.lines[i + 1];
                        if (!IsClassSignature(next_line)) {
                            TOOL::Log::Error("No CLASS corresponding to MCLASS was found:" + EngineHeader.headerName + " Line:" + std::to_string(i + 1));
                            return false;
                        }

                        currentClassName = ExtractClassName(next_line);
						currentClassLineNum = i; // 记录类宏定义所在行数

                        NeedtoFindGenerator = true; // 找到 MCLASS 了，接下来需要找 GENERATE_BODY
                    }

                    if (NeedtoFindGenerator) {
                        if (HasGenerateBody(line)) {
                            NeedtoFindGenerator = false; // 找到 GENERATE_BODY 了，重置状态

                            magicEngineClasss.push_back({EngineHeader.headerName,EngineHeader.moudelName,currentClassName,currentClassLineNum });
                        }
                    }
                }
                if (NeedtoFindGenerator) { // 如果找到了 MCLASS 但是没有找到 GENERATE_BODY 就报错
                    TOOL::Log::Error("No GENERATE_BODY corresponding to MCLASS was found:" + EngineHeader.headerName);
                    return false;
                }
            }

			MagicBuildData::Get().SetMagicEngineClasss(magicEngineClasss);

            return true;
        }
    }
}