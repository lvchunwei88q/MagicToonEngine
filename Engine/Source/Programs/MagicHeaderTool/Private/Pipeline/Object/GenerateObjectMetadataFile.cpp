#include "Pipeline/ObjectPipeline.h"
#include <filesystem>

// Core Libraries
#include <IO.h>							// IO
#include <Jobsystem/Jobsystem.h>		// Jobsystem

namespace Object {
	namespace Pipeline {
		bool GenerateMetadataFile() {
			const auto& MagicObjectMetadatas = MagicObjectBuildData::Get().GetRefMagicObjectMetadatas();

			for (size_t i = 0; i < MagicObjectMetadatas.size(); i++)
			{
				auto& MagicObjectMetadata = MagicObjectMetadatas[i];

				Core::JobSystem::Get().ParallelFor((uint32_t)MagicObjectMetadatas.size(),[&MagicObjectMetadata](uint32_t index) {
					const std::wstring headerName = std::filesystem::path(MagicObjectMetadata.headerName).stem().wstring();
					const std::wstring ModelName = IO::ToWideString(MagicObjectMetadata.moudelName);
					const std::string& Metadata = MagicObjectMetadata.Metadata;
					const std::wstring& GeneratePath = MagicGeneralPipelinedData::Get().GetRefGeneratePath();

					IO::CreateDirectory(GeneratePath + L"\\" + L"Object" + L"\\" + ModelName);
					IO::WriteAllText(GeneratePath + L"\\" + L"Object" + L"\\" + ModelName + L"\\" + headerName + L".object.generate.h", Metadata);
				});
			}
			
			return true;
		}
	}
}