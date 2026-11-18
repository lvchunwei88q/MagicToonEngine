#include "MagicUtilities.h"

// -------------------------------- Pipelines
#include "Pipeline/GeneralPipeline.h"
#include "Pipeline/AbilityPipeline.h"
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

		if (!GeneralPipeline::OutputMagicEngineClassInfo()) {
			Utilitie::Log::Error("An error occurred during the debug data writing stage of the build process");
			return false;
		}
		Utilitie::Log::Info("Pipeline construction successfully, Time during the generation of debug files: " + std::to_string(GTimer.elapsed_ms()) + "ms");
		if (!ctx.IsNeedGenerate())goto end;

		GTimer.start(); // Reset the timer

		if (!GeneralPipeline::GenerateMetadata()) {
			Utilitie::Log::Error("An error occurred during the metadata generation phase of building the pipeline");
			return false;
		}
		Utilitie::Log::Info("Pipeline construction successfully, In metadata generation Time: " + std::to_string(GTimer.elapsed_ms()) + "ms");
		if (!ctx.IsNeedGenerate())goto end;

		GTimer.start(); // Reset the timer

		{	// Ability Metadata Generation
			const bool objectSwitchSucceeded =
				Ability::Pipeline::GenerateObjectSwitchMetadata();
			const bool serializationSucceeded =
				Ability::Pipeline::GenerateSerializationMetadata();
			const bool delegatesSucceeded =
				Ability::Pipeline::GenerateDelegateMetadata();
			if (!objectSwitchSucceeded || !serializationSucceeded || !delegatesSucceeded) {
				Utilitie::Log::Error(
					"One or more ability metadata generators failed; see diagnostics above");
				return false;
			}
			Utilitie::Log::Info("Ability metadata generated in " + std::to_string(GTimer.elapsed_ms()) + "ms");
			if (!ctx.IsNeedGenerate())goto end;
		}

		GTimer.start(); // Reset the timer

		if (!GeneralPipeline::GenerateMetadataFile()) {
			Utilitie::Log::Error("An error occurred while writing generated metadata");
			return false;
		}
		Utilitie::Log::Info("Pipeline construction successfully, In Generate metadata file Time: " + std::to_string(GTimer.elapsed_ms()) + "ms");
		if (!ctx.IsNeedGenerate())goto end;

		GTimer.start(); // Reset the timer
		// -------------------------------------------

	end:
		Utilitie::Log::Info("The current task has been completed.");
		return true;
	}
}
