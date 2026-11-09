#include "Pipeline/AbilityPipeline.h"

#include <algorithm>
#include <sstream>

namespace Ability {
	namespace Pipeline {
		bool GenerateObjectSwitchMetadata()
		{
			const auto& classes = MHT::MagicGeneralPipelinedData::Get().GetRefMagicClasses();
			auto& generatedMetadata = MHT::MagicGeneralPipelinedData::Get().GetRefGeneratedHeaders();
			bool hasErrors = false;

			for (const auto& engineClass : classes) {
				const auto metadata = std::find_if(generatedMetadata.begin(), generatedMetadata.end(),
					[&engineClass](const MHT::GeneratedHeader& candidate) {
						return candidate.headerName == engineClass.headerName;
					});
				if (metadata == generatedMetadata.end()) {
					Utilitie::Log::Error(
						"Could not find generated metadata for " + engineClass.className +
						" in " + engineClass.headerName);
					hasErrors = true;
					continue;
				}

				std::ostringstream code;
				std::vector<std::string> objectSwitches;
				if (std::find(engineClass.mclassParams.begin(), engineClass.mclassParams.end(),
					"MSERIALIZATION") != engineClass.mclassParams.end()) {
					objectSwitches.emplace_back("Core::ObjectSwitch::Serialization");
				}
				if (std::find(engineClass.mclassParams.begin(), engineClass.mclassParams.end(),
					"MDELEGATES") != engineClass.mclassParams.end()) {
					objectSwitches.emplace_back("Core::ObjectSwitch::Delegates");
				}
				// ... Add more object switches based on other class types if needed

				code << "#define GENERATE_OBJECT_SWITCH_" << engineClass.className << " ";
				if (objectSwitches.empty()) {
					code << "Core::ObjectSwitch::Unknown\n";
				}
				else {
					for (size_t i = 0; i < objectSwitches.size(); ++i) {
						if (i > 0) {
							code << " | ";
						}
						code << objectSwitches[i];
					}
					code << "\n";
				}

				metadata->contents += code.str();
			}

			return !hasErrors;
		}
	}
}
