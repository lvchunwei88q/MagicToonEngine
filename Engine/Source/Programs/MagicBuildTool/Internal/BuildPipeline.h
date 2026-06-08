#pragma once
#include <vector>
#include <string>
#include "Tools/Singleton.h"
#include "Tools/Logger.h" 

#define GENERATE_GET(name, type) \
    type Get##name() const { return m_##name; }

// 生成 Set 函数
#define GENERATE_SET(name, type) \
    void Set##name(const type& val) { m_##name = val; }

// 同时生成 Get 和 Set
#define GENERATE_PROPERTY(name, type) \
    GENERATE_GET(name, type) \
    GENERATE_SET(name, type)

namespace MBT {
	// 所有数据都保存在这个单例中，方便在不同的阶段访问和修改
	struct MagicEngineHeader
	{
		std::string headerName;
		std::vector<std::string> lines;

		MagicEngineHeader(const std::string& headerName,const std::vector<std::string>& lines) :headerName(headerName), lines(lines) {}
	};

	struct MagicEngineClass {
		std::string headerName;				// src
		std::string className;				// class
		std::vector<std::string> members;	// members

		MagicEngineClass(const std::string& headerName,const std::string& className) :headerName(headerName), className(className){}
	};

	class MagicBuildData : public Singleton<MagicBuildData>
	{
		std::vector<MagicEngineHeader>	m_MagicEngineHeaders;
		std::vector<MagicEngineClass>	m_MagicEngineClasss;
	public:
		GENERATE_PROPERTY(MagicEngineHeaders, std::vector<MagicEngineHeader>);
		GENERATE_PROPERTY(MagicEngineClasss, std::vector<MagicEngineClass>);

	};

	namespace Pipeline {
		bool FindClass();
	}
};