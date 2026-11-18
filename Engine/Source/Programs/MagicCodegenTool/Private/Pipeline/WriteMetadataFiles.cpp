#include "Pipeline/GeneralPipeline.h"
#include <algorithm>
#include <exception>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

// Core Libraries
#include <IO.h>							// IO
#include <Jobsystem/Jobsystem.h>		// Jobsystem

namespace MCT {
	namespace GeneralPipeline {
		bool GenerateMetadataFile() {
			const auto& generatedMetadata = MagicGeneralPipelinedData::Get().GetRefGeneratedHeaders();
			std::mutex errorsMutex;
			std::vector<std::string> errors;

			Core::JobSystem::Get().ParallelFor(static_cast<uint32_t>(generatedMetadata.size()),
				[&generatedMetadata, &errorsMutex, &errors](uint32_t index) {
					const auto& metadata = generatedMetadata[index];
					try
					{
						const std::wstring headerStem =
							metadata.headerPath.stem().native();
						const IO::path outputDirectory =
							MagicGeneralPipelinedData::Get().GetRefGeneratePath()
							/ L"Object" / IO::ToWideString(metadata.moduleName);
						if (!IO::CreateDirectory(outputDirectory.native()))
						{
							throw std::runtime_error(
								"Could not create generated header directory");
						}

						const IO::path outputPath =
							outputDirectory / (headerStem + L".object.generate.h");
						std::ofstream output(
							outputPath,
							std::ios::binary | std::ios::trunc);
						if (!output)
						{
							throw std::runtime_error("Could not open generated header for writing");
						}
						output.write(
							metadata.contents.data(),
							static_cast<std::streamsize>(metadata.contents.size()));
						output.close();
						if (!output)
						{
							throw std::runtime_error("Could not write generated header");
						}
					}
					catch (const std::exception& error)
					{
						std::lock_guard<std::mutex> lock(errorsMutex);
						errors.push_back(
							"Could not write generated metadata for " +
							metadata.headerPath.string() + ": " + error.what());
					}
				});

			std::sort(errors.begin(), errors.end());
			for (const std::string& error : errors)
			{
				Utilitie::Log::Error(error);
			}
			if (!errors.empty())
			{
				Utilitie::Log::Error(
					"Generated header writing failed for " +
					std::to_string(errors.size()) + " file(s)");
				return false;
			}
			return true;
		}
	}
}
