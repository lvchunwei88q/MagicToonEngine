#include "BuildPipeline.h"
#include <vector>
#include <regex>

namespace MHT {
	namespace {
		struct Result : public MemberVariable {
			bool success = false;
		};
		class MemberVariableParser {
		public:
			static Result Parse(const std::string& decl) {
				Result info;

				// 提取变量名
				info.name = ExtractName(decl);
				if (info.name.empty()) return info;
				// 提取类型
				info.type = ExtractType(decl, info.name);

				info.success = !info.type.empty() && !info.name.empty();
				return info;
			}

		private:
			static std::string ExtractName(const std::string& decl) {
				// 找最后一个标识符
				std::regex namePattern(R"(\b([a-zA-Z_][a-zA-Z0-9_]*)\s*(?:[=;{]|$))");
				std::smatch match;
				std::string::const_iterator searchStart(decl.cbegin());

				std::string lastMatch;
				while (std::regex_search(searchStart, decl.cend(), match, namePattern)) {
					lastMatch = match[1].str();
					searchStart = match[0].second;
				}

				return lastMatch;
			}

			static std::string ExtractType(const std::string& decl, const std::string& name) {
				// 查找变量名，但确保它是一个独立的单词
				std::string searchPattern = "\\b" + name + "\\b";
				std::regex nameRegex(searchPattern);
				std::smatch match;

				if (std::regex_search(decl, match, nameRegex)) {
					size_t namePos = match.position(0);
					std::string typePart = decl.substr(0, namePos);
					// 去除末尾空格
					typePart.erase(typePart.find_last_not_of(" \t") + 1);
					return typePart;
				}

				return "";
			}
		};

		// 检测 MMEMBER(...); 并提取括号内的参数
		bool ParseMMember(const std::string& line, std::vector<std::string>& outParams) {
			outParams.clear();
			// 匹配 MMEMBER( ... ); 允许前后空格，括号内任意字符
			std::regex pattern(R"(^\s*MMEMBER\s*\(\s*([^)]*?)\s*\)\s*;?\s*$)");
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

		bool FindClassMemberInHeader(const MagicEngineHeader& header,std::vector<MemberVariable>& InHeaderMemberVariables) {
			for (size_t i = 0; i < header.lines.size(); i++)
			{
				auto& line = header.lines[i];
				std::vector<std::string> params;
				if (ParseMMember(line, params)) {	// 找到成员变量进入
					if (params.size() > 0) {
						TOOL::Log::Error("Unknown member parameter:" + header.headerName + " Line:" + std::to_string(i + 1));
						return false;
					}

					auto& next_line = header.lines[i + 1];
					auto result = MemberVariableParser::Parse(next_line);
					if (result.success) {
						size_t VariableLineNum = i; // 成员变量宏所在行数 0 start
						InHeaderMemberVariables.push_back({ result.name, result.type,VariableLineNum });
					}
					else {
						TOOL::Log::Error("Undefined member parameter:" + header.headerName + " Line:" + std::to_string(i + 2));
						return false;
					}
				}
			}

			return true;
		}
	}

	namespace Pipeline {
		struct ClassMemberInfo {
			std::vector<MemberVariable> InHeaderMemberVariables;
			std::string headerName;

			ClassMemberInfo(std::vector<MemberVariable>&& InHeaderMemberVariables, std::string headerName)
				: InHeaderMemberVariables(InHeaderMemberVariables), headerName(headerName){}
		};

		struct MagicClassInfo {
			std::vector<MagicEngineClass> MagicClasss;
			std::string headerName;

			MagicClassInfo(std::vector<MagicEngineClass>&& MagicClasss, std::string headerName)
				: MagicClasss(MagicClasss), headerName(headerName) {
			}

			MagicClassInfo(){}
		};

		bool FindClassMember()
		{
			// 首先找到这些类所在的头文件
			const auto& magicEngineClasss = MagicBuildData::Get().GetRefMagicEngineClasss();
			const auto& MagicEngineHeaders = MagicBuildData::Get().GetRefMagicEngineHeaders();
			// Generate Data
			std::vector<MagicEngineHeader> MagicClassHeaders;		// 所有类的所在文件
			std::vector<ClassMemberInfo> MagicClassMemberVariables;	// 存储所有类的成员
			std::vector<MagicClassInfo> MagicOrderClasss;			// 按照顺序存放

			for (size_t i = 0; i < magicEngineClasss.size(); i++)
			{
				auto& magicEngineClass = magicEngineClasss[i];
				for (size_t j = 0; j < MagicEngineHeaders.size(); j++)
				{
					auto& magicEngineHeader = MagicEngineHeaders[j];
					if (magicEngineHeader.headerName == magicEngineClass.headerName) {
						auto it = std::find_if(MagicClassHeaders.begin(), MagicClassHeaders.end(),
							[&magicEngineHeader](const MagicEngineHeader& header) {
								return header.headerName == magicEngineHeader.headerName;
							});
						if (it == MagicClassHeaders.end()) {
							MagicClassHeaders.push_back(magicEngineHeader);
						}
						break;
					}
				}
			}

			// 找到所有的类所在的头文件后，开始在这些头文件中寻找成员变量
			for (size_t i = 0; i < MagicClassHeaders.size(); i++)
			{
				std::vector<MemberVariable> InHeaderMemberVariables;
				auto& MagicClassHeader = MagicClassHeaders[i];
				if (!FindClassMemberInHeader(MagicClassHeader, InHeaderMemberVariables))
					return false;

				MagicClassMemberVariables.push_back({ std::move(InHeaderMemberVariables), MagicClassHeader.headerName });
			}

			// 现在在分类把同一文件的Class放在一起，方便后续处理
			{
				std::vector<std::string> headerNames;
				for (size_t i = 0; i < magicEngineClasss.size(); i++)
				{
					auto& magicEngineClass = magicEngineClasss[i];
					// 检查 headerNames 中是否已经有这个 headerName
					auto it = std::find(headerNames.begin(), headerNames.end(), magicEngineClass.headerName);
					if (it == headerNames.end()) {
						headerNames.push_back(magicEngineClass.headerName);
					}
				}

				for (size_t i = 0; i < headerNames.size(); i++)
				{
					auto& headerName = headerNames[i];

					MagicClassInfo Info;
					Info.headerName = headerName;
					for (size_t x = 0; x < magicEngineClasss.size(); x++)
					{
						auto& magicEngineClass = magicEngineClasss[x];	
						if (headerName == magicEngineClass.headerName) {	// 在这个文件的进入if
							Info.MagicClasss.push_back(magicEngineClass);
						}
					}
					MagicOrderClasss.push_back(Info);
				}
			}

			// 之后给每个文件的同文件中的Class排序按照行数的从小到大
			for (size_t i = 0; i < MagicOrderClasss.size(); i++)
			{
				auto& MagicClass = MagicOrderClasss[i].MagicClasss;
				std::sort(MagicClass.begin(), MagicClass.end(),
					[](const MagicEngineClass& a, const MagicEngineClass& b) {
						return a.lineNum < b.lineNum;
				});

				//for (size_t a = 0; a < MagicClass.size(); a++)
				//	std::cout << MagicClass[a].lineNum << std::endl;
			}
			
			// 给经过排序的class赋值成员
			for (size_t i = 0; i < MagicClassMemberVariables.size(); i++)
			{
				auto& MagicClassMemberVariable = MagicClassMemberVariables[i];

				for (size_t x = 0; x < MagicOrderClasss.size(); x++)
				{
					auto& MagicOrderClass = MagicOrderClasss[x];
					if (MagicClassMemberVariable.headerName == MagicOrderClass.headerName) { // 现在是同一文件的
						auto& MagicClasss = MagicOrderClass.MagicClasss;
						for (size_t y = 0; y < MagicClasss.size(); y++)		// 遍历每个Class
						{
							auto& MagicClass = MagicClasss[y];
							auto* NextMagicClass = y + 1 < MagicClasss.size() ? &MagicClasss[y + 1] : nullptr;
							
							size_t ClassStartLineNum = MagicClass.lineNum; // 开始行数
							size_t ClassMaxEndLineNum = NextMagicClass != nullptr ? NextMagicClass->lineNum : -1 ; // 这里是下一个Class的开始行数但是我们可以判断只要不超过这个值那么成员就是当前类的否则反之

							for (size_t z = 0; z < MagicClassMemberVariable.InHeaderMemberVariables.size(); z++)	// 遍历每个成员
							{
								auto& InHeaderMemberVariable = MagicClassMemberVariable.InHeaderMemberVariables[z];
								size_t VariableLineNum = InHeaderMemberVariable.lineNum; // 当前成员所在的行数

								if (VariableLineNum > ClassMaxEndLineNum || ClassStartLineNum > VariableLineNum)
									continue;	// 不是本类跳过

								// 添加到Class
								MagicClass.members.push_back(InHeaderMemberVariable);
							}
						}
						break;
					}
				}
			}

			std::vector<MagicEngineClass> order_magicEngineClasss;
			// 最后我们重新构造一个MagicEngineClasss
			for (size_t a = 0; a < MagicOrderClasss.size(); a++) {
				auto& MagicOrderClass = MagicOrderClasss[a];
				for (size_t i = 0; i < MagicOrderClass.MagicClasss.size(); i++)
				{
					auto& MagicClass = MagicOrderClass.MagicClasss[i];
					order_magicEngineClasss.push_back(MagicClass);
				}
			}
			MagicBuildData::Get().SetMagicEngineClasss(order_magicEngineClasss);

#if 0
			for (size_t a = 0; a < order_magicEngineClasss.size(); a++) {
				auto& OrderClass = order_magicEngineClasss[a];
				std::cout << OrderClass.className << std::endl;
				for (size_t i = 0; i < OrderClass.members.size(); i++)
				{
					auto& members = OrderClass.members[i];
					std::cout << "   " << members.type << " - ";
					std::cout << members.name << std::endl;
				}
			}
#endif

			return true;
		}
	}
};