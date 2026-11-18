#pragma once

#include <vector>
#include <string>
#include <utility>
#include <Utilities/Singleton.h>
#include <IO.h>
#include "MagicUtilities.h" 

namespace MCT {
	// All the data is stored in this singleton, making it easy to access and modify at different stages.
	struct MagicEngineHeader
	{
		IO::path headerPath;	// The path to the header file
		std::string moduleName;
		std::vector<std::string> lines;

		MagicEngineHeader(const IO::path& headerPath, const std::string& moduleName, const std::vector<std::string>& lines)
			: headerPath(headerPath), moduleName(moduleName), lines(lines) {}
		MagicEngineHeader() {}
	};

	// All values used to record row numbers start from 0 by default.
	struct MemberVariable {
		std::string name;			// Variable name
		std::string type;			// Variable type
		std::string access;			// Access specifier: public, protected, private
		std::vector<std::string> annotationParams;	// Parameters for MMEMBER macro
		bool hasMMember = false;
		size_t lineNum = 0;			// indicates the line number of the member variable declaration, starting from 0

		MemberVariable(std::string name, std::string type, size_t lineNum,
			std::string access = {})
			: name(name), type(type), access(access), lineNum(lineNum) {}
		MemberVariable() {}
	};

	struct MagicClassFunction {
		std::string name;			// Function name
		std::string signature;		// Function signature, including return type and parameters
		std::string access;			// Access specifier: public, protected, private
		std::vector<std::string> annotationParams;	// Parameters for MFUNCTION macro
		bool hasMFunction = false;
		size_t lineNum = 0;

		MagicClassFunction(
			std::string name,
			std::string signature,
			std::string access,
			size_t lineNum)
			: name(std::move(name)),
			signature(std::move(signature)),
			access(std::move(access)),
			lineNum(lineNum) {}

		MagicClassFunction() = default;
	};

	struct MagicClassInfo {
		std::string className;
		std::string qualifiedName;
		std::string kind;
		IO::path headerPath;
		std::string moduleName;
		size_t lineNum = 0;				// Zero-based class declaration line
		size_t endLineNum = 0;			// Zero-based class end line
		std::vector<std::string> mclassParams;
		std::vector<std::string> generateBodyParams;
		std::vector<std::string> baseClasses;
		std::vector<MemberVariable> members;	// Member declaration lineNum values are zero-based
		std::vector<MagicClassFunction> functions;
	};

	struct GeneratedHeader {
		IO::path headerPath;
		std::string contents;
		std::string moduleName;
	};

	class MagicGeneralPipelinedData : public Singleton<MagicGeneralPipelinedData>
	{
		std::vector<MagicEngineHeader>		m_MagicEngineHeaders;
		std::vector<MagicClassInfo>			m_MagicClasses;
		std::vector<GeneratedHeader>		m_GeneratedHeaders;
		std::vector<std::string>			m_AllUndefMacros = { "GetClassName" };
		IO::path m_GeneratePath;
	public:
		GENERATE_PROPERTY(MagicEngineHeaders, std::vector<MagicEngineHeader>);
		GENERATE_PROPERTY(MagicClasses, std::vector<MagicClassInfo>);
		GENERATE_PROPERTY(GeneratedHeaders, std::vector<GeneratedHeader>);
		GENERATE_PROPERTY(GeneratePath, IO::path);
		GENERATE_GET_REF(AllUndefMacros, std::vector<std::string>);

		void clearAll() {
			m_MagicEngineHeaders.clear();
			m_MagicClasses.clear();
			m_GeneratedHeaders.clear();
		}
	};
	using MagicGPD = decltype(MagicGeneralPipelinedData::Get());

	namespace GeneralPipeline {
		bool FindEngineClass();
		bool FindClassMember();
		bool OutputMagicEngineClassInfo();
		bool GenerateMetadata();
		bool GenerateMetadataFile();

		bool RunBuildPipeline(); // for object build pipeline
	}

}
