#include <FBXImport.h>
#include <ufbx.h>
#include <FileManager.h>
#include <Converter.h>
#include <ILog.h>

namespace EngineScene {
	void FBXAssetImport::Import(FBXImportSettings settings)
	{
		// TODO: Import FBX Function

		if (settings.path.empty())
		{
			LOG_ERROR("FBX Import failed: file path is empty");
			return;
		}
		if (!IO::FileManager::Exists(IO::Converter::ToWideString(settings.path)))
		{
			LOG_ERROR("FBX Import failed: file does not exist - %s", settings.path.c_str());
			return;
		}

		ufbx_load_opts opts = { 0 };  // ufbx 加载选项
		ufbx_error error;             // 错误信息

		ufbx_scene* scene = ufbx_load_file(settings.path.c_str(), &opts, &error);

		if (!scene)
		{
			LOG_ERROR("FBX Import failed: %s", error.description.data);
			return;
		}

		LOG_INFO("FBX file loaded successfully: " + settings.path);

		LOG_INFO("========== FBX Scene Summary ==========");
		LOG_INFO("Nodes:               " + std::to_string(scene->nodes.count));
		LOG_INFO("Meshes:              " + std::to_string(scene->meshes.count));
		LOG_INFO("Materials:           " + std::to_string(scene->materials.count));
		LOG_INFO("Textures:            " + std::to_string(scene->textures.count));
		LOG_INFO("Textures Files:      " + std::to_string(scene->texture_files.count));
		LOG_INFO("Bones:               " + std::to_string(scene->bones.count));
		LOG_INFO("Empties:             " + std::to_string(scene->empties.count));
		LOG_INFO("Anim Stacks:         " + std::to_string(scene->anim_stacks.count));
		LOG_INFO("Anim Layers:         " + std::to_string(scene->anim_layers.count));
		LOG_INFO("Anim Curves:         " + std::to_string(scene->anim_curves.count));
		LOG_INFO("Skin Deformers:      " + std::to_string(scene->skin_deformers.count));
		LOG_INFO("Skin Clusters:       " + std::to_string(scene->skin_clusters.count));
		LOG_INFO("Blend Deformers:     " + std::to_string(scene->blend_deformers.count));
		LOG_INFO("Blend Shapes:        " + std::to_string(scene->blend_shapes.count));
		//LOG_INFO("Poses:               " + std::to_string(scene->poses.count));
		LOG_INFO("========================================");


		ufbx_free_scene(scene);
	}
}