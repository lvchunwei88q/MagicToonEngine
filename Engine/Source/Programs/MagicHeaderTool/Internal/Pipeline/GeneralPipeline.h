#pragma once

#include <vector>
#include <string>
#include <Utilities/Singleton.h>
#include "MagicUtilities.h" 

namespace MHT {
	// 所有数据都保存在这个单例中，方便在不同的阶段访问和修改
	struct MagicEngineHeader
	{
		std::string headerName;
		std::string moudelName;
		std::vector<std::string> lines;

		MagicEngineHeader(const std::string& headerName, const std::vector<std::string>& lines)
			: headerName(headerName), lines(lines) {}
		MagicEngineHeader() {}
	};

	// 所有用于记录行数的值我们默认都是从0开始的，方便计算和使用
	struct MemberVariable {
		std::string name;			// 名称
		std::string type;			// 类型
		size_t lineNum;				// 在头文件中的行数

		MemberVariable(std::string name, std::string type, size_t lineNum) : name(name), type(type), lineNum(lineNum) {}
		MemberVariable() {}
	};

	struct MagicEngineClass {
		std::vector<MemberVariable> members;	// members
		std::vector<std::string> ClassType;		// class Required function
		std::vector<std::string> GenerateBody;	// class Required function
		std::string headerName;					// src
		std::string moudelName;					// model
		std::string className;					// class
		size_t lineNum;							// line number in header file

		MagicEngineClass(const std::vector<std::string>& ClassType,
			const std::vector<std::string>& GenerateBody,
			const std::string& headerName,
			const std::string& moudelName,
			const std::string& className,
			size_t lineNum)
			: members(), ClassType(ClassType), GenerateBody(GenerateBody),
			headerName(headerName), moudelName(moudelName),
			className(className), lineNum(lineNum) {}

		MagicEngineClass() {}
	};

	class MagicGeneralPipelinedData : public Singleton<MagicGeneralPipelinedData>
	{
		std::vector<MagicEngineHeader>		m_MagicEngineHeaders;
		std::vector<MagicEngineClass>		m_MagicEngineClasss;

		std::wstring m_GeneratePath;
	public:
		GENERATE_PROPERTY(MagicEngineHeaders, std::vector<MagicEngineHeader>);
		GENERATE_PROPERTY(MagicEngineClasss, std::vector<MagicEngineClass>);
		GENERATE_PROPERTY(GeneratePath, std::wstring);

		void clearAll() {
			m_MagicEngineHeaders.clear();
			m_MagicEngineClasss.clear();
		}
	};
	using MagicGPD = decltype(MagicGeneralPipelinedData::Get());

	namespace GeneralPipeline {
		bool FindEngineClass();
		bool FindClassMember();

		bool RunBuildPipeline(); // for object build pipeline
	}

}