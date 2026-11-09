#include "Pipeline/AbilityPipeline.h"

#include <algorithm>

namespace Ability {
	namespace Pipeline {
		bool GenerateDelegateMetadata()
		{
			const auto& classes = MHT::MagicGeneralPipelinedData::Get().GetRefMagicClasses();
			auto& generatedMetadata = MHT::MagicGeneralPipelinedData::Get().GetRefGeneratedHeaders();

			for (const auto& engineClass : classes) {
				if (!engineClass.hasMClass) {
					continue;
				}
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

				const bool hasDelegates = std::find(engineClass.mclassParams.begin(), engineClass.mclassParams.end(),
					"MDELEGATES") != engineClass.mclassParams.end();

				// By default, add an empty implementation if this class has this capability enabled to override
				metadata->contents += "#define GENERATE_DELEGATES_" + engineClass.className + "()";
				if (hasDelegates) {
					// Add delegate-specific generated code here when that capability is implemented.
				}
				metadata->contents += "\n\n";
			}

			return true;
		}
	}
}
