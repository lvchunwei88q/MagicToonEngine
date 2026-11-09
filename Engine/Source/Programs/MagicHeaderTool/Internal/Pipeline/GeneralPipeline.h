#pragma once

#include <vector>
#include <string>
#include <Utilities/Singleton.h>
#include "MagicUtilities.h" 

namespace MHT {
	// All the data is stored in this singleton, making it easy to access and modify at different stages.
	struct MagicEngineHeader
	{
		std::string headerName;
		std::string moudelName;
		std::vector<std::string> lines;

		MagicEngineHeader(const std::string& headerName, const std::vector<std::string>& lines)
			: headerName(headerName), lines(lines) {}
		MagicEngineHeader() {}
	};

	// All values used to record row numbers start from 0 by default.
	struct MemberVariable {
		std::string name;			// 名称
		std::string type;			// 类型
		std::string access;
		std::vector<std::string> annotationParams;
		bool hasMMember = false;
		size_t lineNum = 0;			// 在头文件中的行数

		MemberVariable(std::string name, std::string type, size_t lineNum,
			std::string access = {})
			: name(name), type(type), access(access), lineNum(lineNum) {}
		MemberVariable() {}
	};

	struct MagicClassFunction {
		std::string name;
		std::string signature;
		std::string access;
		size_t lineNum = 0;
	};

	struct MagicClassInfo {
		std::string className;
		std::string qualifiedName;
		std::string kind;
		std::string headerName;
		std::string moduleName;
		size_t lineNum = 0;				// Zero-based class declaration line
		size_t endLineNum = 0;			// Zero-based class end line
		bool isDefinition = false;
		bool hasMClass = false;
		bool hasGenerateBody = false;
		std::vector<std::string> mclassParams;
		std::vector<std::string> generateBodyParams;
		std::vector<std::string> baseClasses;
		std::vector<MemberVariable> members;	// Member declaration lineNum values are zero-based
		std::vector<MagicClassFunction> functions;
	};

	struct GeneratedHeader {
		std::string headerName;
		std::string contents;
		std::string moduleName;
	};

	class MagicGeneralPipelinedData : public Singleton<MagicGeneralPipelinedData>
	{
		std::vector<MagicEngineHeader>		m_MagicEngineHeaders;
		std::vector<MagicClassInfo>			m_MagicClasses;
		std::vector<GeneratedHeader>		m_GeneratedHeaders;
		std::vector<std::string>			m_AllUndefMacros = { "GetClassName" };

		std::wstring m_GeneratePath;
		std::string m_CompileCommandsPath;
	public:
		GENERATE_PROPERTY(MagicEngineHeaders, std::vector<MagicEngineHeader>);
		GENERATE_PROPERTY(MagicClasses, std::vector<MagicClassInfo>);
		GENERATE_PROPERTY(GeneratedHeaders, std::vector<GeneratedHeader>);
		GENERATE_PROPERTY(GeneratePath, std::wstring);
		GENERATE_PROPERTY(CompileCommandsPath, std::string);
		GENERATE_GET_REF(AllUndefMacros, std::vector<std::string>);

		void clearAll() {
			m_MagicEngineHeaders.clear();
			m_MagicClasses.clear();
			m_GeneratedHeaders.clear();
		}
	};
	using MagicGPD = decltype(MagicGeneralPipelinedData::Get());

	namespace GeneralPipeline {
		bool FindClass();
		bool FindEngineClass();
		bool FindClassMember();
		bool OutputMagicEngineClassInfo();
		bool GenerateMetadata();
		bool GenerateMetadataFile();

		bool RunBuildPipeline(); // for object build pipeline
	}

}
