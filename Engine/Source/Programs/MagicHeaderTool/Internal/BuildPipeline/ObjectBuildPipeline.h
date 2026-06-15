#pragma once
#include "BuildPipeline.h"
using namespace MHT;
namespace Object {

	struct MagicObjectMetadata {
		std::string headerName;
		std::string Metadata;
		std::string moudelName;

		MagicObjectMetadata(const std::string& headerName,
			const std::string& Metadata,
			const std::string& moudelName)
			: headerName(headerName), Metadata(Metadata), moudelName(moudelName) {}

		MagicObjectMetadata() {}
	};

	class MagicObjectBuildData : public Singleton<MagicObjectBuildData>
	{
		std::vector<MagicObjectMetadata>	m_MagicObjectMetadatas;
	public:
		GENERATE_PROPERTY(MagicObjectMetadatas, std::vector<MagicObjectMetadata>);

		void clear() {
			m_MagicObjectMetadatas.clear();
		}
	};

	namespace Pipeline {
		bool GenerateObjectMetadata();
		bool GenerateMetadataFile();
	}
};