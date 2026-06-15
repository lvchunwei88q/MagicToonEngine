#include "BuildPipeline/ObjectBuildPipeline.h"
#include <filesystem>
#include "Tools/Jobsystem.h"		// Jobsystem
#include "Tools/Tool.h"				// IO

namespace Object {
	namespace Pipeline {
		bool GenerateMetadataFile() {
			// TODO 优化多个构建管线文件生成
			const auto& MagicObjectMetadatas = MagicObjectBuildData::Get().GetRefMagicObjectMetadatas();

			for (size_t i = 0; i < MagicObjectMetadatas.size(); i++)
			{
				auto& MagicObjectMetadata = MagicObjectMetadatas[i];

				JobSystem::Get().ParallelFor((uint32_t)MagicObjectMetadatas.size(),[&MagicObjectMetadata](uint32_t index) {
					const std::wstring headerName = std::filesystem::path(MagicObjectMetadata.headerName).stem().wstring();
					const std::wstring ModelName = TOOL::ToWideString(MagicObjectMetadata.moudelName);
					const std::string& Metadata = MagicObjectMetadata.Metadata;
					const std::wstring& GeneratePath = MagicBuildData::Get().GetRefGeneratePath();

					TOOL::MakeDirectory(GeneratePath + L"\\" + ModelName);
					TOOL::WriteAllText(GeneratePath + L"\\" + ModelName + L"\\" + headerName + L".generate.h", Metadata);
				});
			}
			
			return true;
		}
	}
}