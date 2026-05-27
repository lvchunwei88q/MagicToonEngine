#include "FBXImport.h"
#include <ufbx.h>
#include <IO.h> 
#include <ILog.h>

namespace RuntimeAssets {
	void FBXAssetImport::Import(FBXImportSettings settings)
	{
		// TODO: Import FBX Function

		if (settings.path.empty())
		{
			LOG_ERROR("FBX Import failed: file path is empty");
			return;
		}
		if (!IO::Exists(IO::ToWideString(settings.path)))
		{
			LOG_ERROR("FBX Import failed: file does not exist - " , settings.path);
			return;
		}

		ufbx_load_opts opts = { 0 };  // ufbx 加载选项
		ufbx_error error;             // 错误信息

		ufbx_scene* scene = ufbx_load_file(settings.path.c_str(), &opts, &error);

		if (!scene)
		{
			LOG_ERROR("FBX Import failed: ", error.description.data);
			return;
		}

		LOG_INFO("FBX file loaded successfully: " + settings.path);

		LOG_INFO("========== FBX Scene Summary ==========");
		LOG_INFO("Nodes:               ", scene->nodes.count);
		LOG_INFO("Meshes:              ", scene->meshes.count);
		LOG_INFO("Materials:           ", scene->materials.count);
		LOG_INFO("Textures:            ", scene->textures.count);
		LOG_INFO("Textures Files:      ", scene->texture_files.count);
		LOG_INFO("Bones:               ", scene->bones.count);
		LOG_INFO("Empties:             ", scene->empties.count);
		LOG_INFO("Anim Stacks:         ", scene->anim_stacks.count);
		LOG_INFO("Anim Layers:         ", scene->anim_layers.count);
		LOG_INFO("Anim Curves:         ", scene->anim_curves.count);
		LOG_INFO("Skin Deformers:      ", scene->skin_deformers.count);
		LOG_INFO("Skin Clusters:       ", scene->skin_clusters.count);
		LOG_INFO("Blend Deformers:     ", scene->blend_deformers.count);
		LOG_INFO("Blend Shapes:        ", scene->blend_shapes.count);
		//LOG_INFO("Poses:               " + std::to_string(scene->poses.count));
		LOG_INFO("========================================");


		ufbx_free_scene(scene);
	}
}