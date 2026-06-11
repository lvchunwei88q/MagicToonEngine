#pragma once
#include <vector>
#include <string>
#include "Tools/Singleton.h"
#include "Tools/Tool.h" 

#define GENERATE_GET(name, type) \
    type Get##name() const { return m_##name; }

#define GENERATE_GET_REF(name, type) \
    const type& GetRef##name() const { return m_##name; } \
    type& GetRef##name() { return m_##name; }

#define GENERATE_SET(name, type) \
    void Set##name(const type& val) { m_##name = val; }

// 同时生成 Get 和 Set
#define GENERATE_PROPERTY(name, type)	\
    GENERATE_GET(name, type)			\
	GENERATE_GET_REF(name, type)		\
    GENERATE_SET(name, type)

namespace MHT {
	// 所有数据都保存在这个单例中，方便在不同的阶段访问和修改
	struct MagicEngineHeader
	{
		std::string headerName;
		std::string moudelName;
		std::vector<std::string> lines;

		MagicEngineHeader(const std::string& headerName,const std::vector<std::string>& lines)
																: headerName(headerName), lines(lines) {}
		MagicEngineHeader(){}
	};

	// 所有用于记录行数的值我们默认都是从0开始的，方便计算和使用

	struct MemberVariable {
		std::string name;			// 名称
		std::string type;			// 类型
		size_t lineNum;				// 在头文件中的行数

		MemberVariable(std::string name, std::string type,size_t lineNum) : name(name), type(type), lineNum(lineNum) {}
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
			: members(),ClassType(ClassType), GenerateBody(GenerateBody),
			headerName(headerName), moudelName(moudelName),
			className(className), lineNum(lineNum){}

		MagicEngineClass(){}
	};

	struct MagicObjectMetadata {
		std::string headerName;
		std::string Metadata;
		std::string moudelName;

		MagicObjectMetadata(const std::string& headerName,
							const std::string& Metadata,
							const std::string& moudelName)
			: headerName(headerName), Metadata(Metadata), moudelName(moudelName){}

		MagicObjectMetadata() {}
	};

	class MagicBuildData : public Singleton<MagicBuildData>
	{
		std::vector<MagicEngineHeader>		m_MagicEngineHeaders;
		std::vector<MagicEngineClass>		m_MagicEngineClasss;
		std::vector<MagicObjectMetadata>	m_MagicObjectMetadatas;

		std::wstring m_GeneratePath;
	public:
		GENERATE_PROPERTY(MagicEngineHeaders,	std::vector<MagicEngineHeader>		);
		GENERATE_PROPERTY(MagicEngineClasss,	std::vector<MagicEngineClass>		);
		GENERATE_PROPERTY(MagicObjectMetadatas, std::vector<MagicObjectMetadata>	);
		GENERATE_PROPERTY(GeneratePath,			std::wstring						);

		void clear() {
			m_MagicEngineHeaders.clear();
			m_MagicEngineClasss.clear();
			m_MagicObjectMetadatas.clear();
		}

	};

	namespace Pipeline {
		enum GenerationState {
			Pending,       // 有待生成的数据
			Completed      // 已无待生成数据
		};
		extern GenerationState GenerationState_;
		inline bool NeedGenerate()			{ return GenerationState_ == GenerationState::Pending;	}
		inline void SetGenerateCompleted()	{ GenerationState_ = GenerationState::Completed;		}
		inline void ResetStatus()			{ GenerationState_ = GenerationState::Pending;			}

		bool FindEngineClass();
		bool FindClassMember();
		bool GenerateObjectMetadata();
		bool GenerateMetadataFile();
	}
};