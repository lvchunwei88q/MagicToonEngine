#include "MagicUtilities.h"

// -------------------------------- Pipelines
#include "Pipeline/GeneralPipeline.h"
#include "Pipeline/ObjectPipeline.h"
// --------------------------------

namespace MHT{

	bool GeneralPipeline::RunBuildPipeline()
	{
		MGC ctx = MagicGlobalContext::Get();
		Utilitie::GTimer GTimer = Utilitie::GlobalTimer::Get();
		GTimer.start(); // Reset the timer

		if (!GeneralPipeline::FindEngineClass()) {
			Utilitie::Log::Error("An error occurred during the Class-finding phase of the build pipeline");
			return false;
		}
		Utilitie::Log::Info("Pipeline construction successfully, In found Class stage Time: " + std::to_string(GTimer.elapsed_ms()) + "ms");
		if (!ctx.IsNeedGenerate())goto end;
		
		GTimer.start(); // Reset the timer

		if (!GeneralPipeline::FindClassMember()) {
			Utilitie::Log::Error("An error occurred during the Class Member-finding phase of the build pipeline");
			return false;
		}
		Utilitie::Log::Info("Pipeline construction successfully, In found Class Member stage Time: " + std::to_string(GTimer.elapsed_ms()) + "ms");
		if (!ctx.IsNeedGenerate())goto end;
		
		GTimer.start(); // Reset the timer

		// ------------------------------------------- 

		{ // Object
			if (!Object::Pipeline::GenerateObjectMetadata()) {
				Utilitie::Log::Error("An error occurred during the Object metadata generation phase of building the pipeline");
				return false;
			}
			Utilitie::Log::Info("Pipeline construction successfully, In Generate Object Metadata Time: " + std::to_string(GTimer.elapsed_ms()) + "ms");
			if (!ctx.IsNeedGenerate())goto end;
			
			GTimer.start(); // Reset the timer

			if (!Object::Pipeline::GenerateMetadataFile()) {
				Utilitie::Log::Error("An error occurred during the Class Member-finding phase of the build pipeline");
				return false;
			}
			Utilitie::Log::Info("Pipeline construction successfully, In Generate metadata file Time: " + std::to_string(GTimer.elapsed_ms()) + "ms");
			if (!ctx.IsNeedGenerate())goto end;
			
			GTimer.start(); // Reset the timer
		}

		// -------------------------------------------

	end:
		return true;
	}
}