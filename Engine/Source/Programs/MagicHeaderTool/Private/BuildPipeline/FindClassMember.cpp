#include "BuildPipeline.h"
#include <vector>
#include <ctre/ctre.hpp>
#include <algorithm>

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
				std::string_view view(decl);
				std::string lastMatch;

				// 使用 CTRE 的 search 获取匹配
				auto matches = ctre::search_all<R"(\b([a-zA-Z_][a-zA-Z0-9_]*)\s*(?:[=;{]|$))">(view);
				for (auto& match : matches) {
					// 获取第一个捕获组的内容
					if (match.get<1>().size() > 0) {
						lastMatch = match.get<1>().to_string();
					}
				}

				return lastMatch;
			}

			static std::string ExtractType(const std::string& decl, const std::string& name) {
				// 使用 CTRE 查找变量名
				std::string pattern = "\\b" + name + "\\b";
				// CTRE 要求编译期确定的字符串，所以不能使用运行时拼接的 pattern 用 string_view 手动查找
				std::string_view view(decl);
				size_t pos = 0;
				while (pos < view.size()) {
					// 查找 name 字符串
					size_t name_pos = view.find(name, pos);
					if (name_pos == std::string_view::npos) break;
					// 检查是否为独立单词
					bool is_word_boundary = true;
					// 检查前一个字符
					if (name_pos > 0) {
						char prev = view[name_pos - 1];
						if (std::isalnum(static_cast<unsigned char>(prev)) || prev == '_') {
							is_word_boundary = false;
						}
					}
					// 检查后一个字符
					if (is_word_boundary && name_pos + name.size() < view.size()) {
						char next = view[name_pos + name.size()];
						if (std::isalnum(static_cast<unsigned char>(next)) || next == '_') {
							is_word_boundary = false;
						}
					}

					if (is_word_boundary) {
						std::string typePart = std::string(view.substr(0, name_pos));
						while (!typePart.empty() && (typePart.back() == ' ' || typePart.back() == '\t')) {
							typePart.pop_back();
						}
						return typePart;
					}

					pos = name_pos + name.size();
				}

				return "";
			}
		};

		// 检测 MMEMBER(...); 并提取括号内的参数
		bool ParseMMember(const std::string& line, std::vector<std::string>& outParams) {
			outParams.clear();

			// 匹配 MMEMBER( ... ); 允许前后空格，括号内任意字符
			if (auto match = ctre::match<R"(^\s*MMEMBER\s*\(\s*([^)]*?)\s*\)\s*;?\s*$)">(line)) {
				std::string_view args = match.get<1>();

				std::string_view remaining = args;
				while (!remaining.empty()) {
					// 查找逗号位置
					size_t comma_pos = remaining.find(',');
					std::string_view param = (comma_pos == std::string_view::npos)
						? remaining
						: remaining.substr(0, comma_pos);

					size_t start = param.find_first_not_of(" \t");
					if (start == std::string_view::npos) {
						// 全是空白，跳过这个参数
						remaining = (comma_pos == std::string_view::npos)
							? std::string_view()
							: remaining.substr(comma_pos + 1);
						continue;
					}
					size_t end = param.find_last_not_of(" \t");
					param = param.substr(start, end - start + 1);

					if (!param.empty()) {
						outParams.emplace_back(param);
					}
					remaining = (comma_pos == std::string_view::npos)
						? std::string_view()
						: remaining.substr(comma_pos + 1);
				}
				return true;
			}
			return false;
		}

		bool FindClassMemberInHeader(const MagicEngineHeader& header,std::vector<MemberVariable>& InHeaderMemberVariables) 
		{
			for (size_t i = 0; i < header.lines.size(); i++)
			{
				auto& line = header.lines[i];
				std::vector<std::string> params;
				if (ParseMMember(line, params)) {	// 找到成员变量进入
					if (params.size() > 0) {
						TOOL::Log::Error("Unknown member parameter:" + header.headerName + " Line:" + std::to_string(i + 1));
						return false;
					}

					if (i + 1 >= header.lines.size()) {
						TOOL::Log::Error("MMEMBER at end of file without declaration: " + header.headerName + " Line:" + std::to_string(i + 1));
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