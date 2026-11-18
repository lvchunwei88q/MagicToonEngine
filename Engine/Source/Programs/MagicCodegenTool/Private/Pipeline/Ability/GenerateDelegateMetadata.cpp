#include "Pipeline/AbilityPipeline.h"

#include <algorithm>

namespace Ability {
	namespace Pipeline {
		bool GenerateDelegateMetadata()
		{
			const auto& classes = MCT::MagicGeneralPipelinedData::Get().GetRefMagicClasses();
			auto& generatedMetadata = MCT::MagicGeneralPipelinedData::Get().GetRefGeneratedHeaders();
			bool hasErrors = false;

			for (const auto& engineClass : classes) {
				const auto metadata = std::find_if(generatedMetadata.begin(), generatedMetadata.end(),
					[&engineClass](const MCT::GeneratedHeader& candidate) {
						return candidate.headerPath == engineClass.headerPath;
					});
				if (metadata == generatedMetadata.end()) {
					Utilitie::Log::Error(
						"Could not find generated metadata for " + engineClass.className +
						" in " + engineClass.headerPath.string());
					hasErrors = true;
					continue;
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

			return !hasErrors;
		}
	}
}
