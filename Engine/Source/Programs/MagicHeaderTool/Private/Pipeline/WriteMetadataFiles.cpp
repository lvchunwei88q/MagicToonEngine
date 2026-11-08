#include "Pipeline/GeneralPipeline.h"
#include <filesystem>

// Core Libraries
#include <IO.h>							// IO
#include <Jobsystem/Jobsystem.h>		// Jobsystem

namespace MHT {
	namespace GeneralPipeline {
		bool GenerateMetadataFile() {
			const auto& generatedMetadata = MagicGeneralPipelinedData::Get().GetRefGeneratedHeaders();

			Core::JobSystem::Get().ParallelFor(static_cast<uint32_t>(generatedMetadata.size()),
				[&generatedMetadata](uint32_t index) {
					const auto& metadata = generatedMetadata[index];
					const std::wstring headerName = std::filesystem::path(metadata.headerName).stem().wstring();
					const std::wstring moduleName = IO::ToWideString(metadata.moduleName);
					const std::wstring& generatePath = MagicGeneralPipelinedData::Get().GetRefGeneratePath();

					IO::CreateDirectory(generatePath + L"\\Object\\" + moduleName);
					IO::WriteAllText(
						generatePath + L"\\Object\\" + moduleName + L"\\" + headerName + L".object.generate.h",
						metadata.contents);
				});
			
			return true;
		}
	}
}