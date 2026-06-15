#include "BuildPipeline/BuildPipeline.h"
#include "Tools/Tool.h"

// -------------------------------- Pipelines
#include "BuildPipeline/ObjectBuildPipeline.h"
// --------------------------------

namespace MHT{
	namespace Pipeline {
		GenerationState GenerationState_;
	}

	namespace {
		TOOL::Timer timer; // 计时器
	}

	bool Pipeline::RunBuildPipeline()
	{
		timer.reset(); // 重置计时器

		if (!Pipeline::FindEngineClass()) {
			TOOL::Log::Error("An error occurred during the Class-finding phase of the build pipeline");
			return false;
		}
		TOOL::Log::Info("Pipeline construction successfully, In found Class stage Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		if (!Pipeline::NeedGenerate())goto end;
		timer.reset(); // 重置计时器

		if (!Pipeline::FindClassMember()) {
			TOOL::Log::Error("An error occurred during the Class Member-finding phase of the build pipeline");
			return false;
		}
		TOOL::Log::Info("Pipeline construction successfully, In found Class Member stage Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		if (!Pipeline::NeedGenerate())goto end;
		timer.reset(); // 重置计时器

		// ------------------------------------------- 

		if (!Object::Pipeline::GenerateObjectMetadata()) {
			TOOL::Log::Error("An error occurred during the Object metadata generation phase of building the pipeline");
			return false;
		}
		TOOL::Log::Info("Pipeline construction successfully, In Generate Object Metadata Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		if (!Pipeline::NeedGenerate())goto end;
		timer.reset(); // 重置计时器

		if (!Object::Pipeline::GenerateMetadataFile()) {
			TOOL::Log::Error("An error occurred during the Class Member-finding phase of the build pipeline");
			return false;
		}

		// -------------------------------------------

		TOOL::Log::Info("Pipeline construction successfully, In Generate metadata file Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		if (!Pipeline::NeedGenerate())goto end;
		timer.reset(); // 重置计时器

	end:
		return true;
	}
}