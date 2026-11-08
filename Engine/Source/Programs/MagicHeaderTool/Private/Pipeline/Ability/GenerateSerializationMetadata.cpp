#include "Pipeline/AbilityPipeline.h"

#include <algorithm>
#include <sstream>

namespace Ability {
	namespace Pipeline {
		bool GenerateSerializationMetadata()
		{
			const auto& classes = MHT::MagicGeneralPipelinedData::Get().GetRefMagicEngineClasss();
			auto& generatedMetadata = MHT::MagicGeneralPipelinedData::Get().GetRefGeneratedHeaders();

			for (const auto& engineClass : classes) {
				const auto metadata = std::find_if(generatedMetadata.begin(), generatedMetadata.end(),
					[&engineClass](const MHT::GeneratedHeader& candidate) {
						return candidate.headerName == engineClass.headerName;
					});
				if (metadata == generatedMetadata.end()) {
					Utilitie::Log::Error(
						"Could not find generated metadata for " + engineClass.className +
						" in " + engineClass.headerName);
					return false;
				}

				std::ostringstream code;
				// By default, add an empty implementation if this class has this capability enabled to override
				code << "#define GENERATE_SERIALIZE_" << engineClass.className << "() \\\n\n";
				code << "#define GENERATE_SERIALIZE_BODY_" << engineClass.className << "() \\\n\n";
				metadata->contents += code.str();

				const bool hasSerialization = std::find(engineClass.ClassType.begin(), engineClass.ClassType.end(),
					"MSERIALIZATION") != engineClass.ClassType.end();

				if(hasSerialization){
					code << "#ifdef GENERATE_SERIALIZE_" << engineClass.className << "\n";
					code << "#undef GENERATE_SERIALIZE_" << engineClass.className << "\n";
					code << "#endif\n";
					code << "#define GENERATE_SERIALIZE_" << engineClass.className << "() \\\n";
					code << "    template<class Archive> \\\n";
					if (engineClass.members.empty()) {
						code << "    void serialize(Archive&) {} \\\n";
					}
					else {
						code << "    void serialize(Archive& archive) { \\\n";
						code << "        archive(";
						for (size_t i = 0; i < engineClass.members.size(); ++i) {
							code << engineClass.members[i].name;
							if (i + 1 < engineClass.members.size()) {
								code << ", ";
							}
						}
						code << "); \\\n";
						code << "    } \\\n";
					}
					code << "\n";

					code << "#ifdef GENERATE_SERIALIZE_BODY_" << engineClass.className << "\n";
					code << "#undef GENERATE_SERIALIZE_BODY_" << engineClass.className << "\n";
					code << "#endif\n";
					code << "#define GENERATE_SERIALIZE_BODY_" << engineClass.className << "() \\\n";
					code << "    virtual void SerializeData(std::ostringstream& stream) override { \\\n";
					code << "        cereal::BinaryOutputArchive archive(stream); \\\n";
					code << "        archive(*this); \\\n";
					code << "    } \\\n";
					code << "    virtual void DeserializeData(std::stringstream& stream) override { \\\n";
					code << "        cereal::BinaryInputArchive archive(stream); \\\n";
					code << "        archive(*this); \\\n";
					code << "    }\n\n";

					metadata->contents += code.str();
				}
				metadata->contents += "\n\n";
			}

			return true;
		}
	}
}
