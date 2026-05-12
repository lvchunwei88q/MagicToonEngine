#pragma once
#include <string>
#include <RenderSubsystem/RenderSubsystem.h>
#include <cereal/cereal.hpp>
#include <EditorUIWarehouse.h>

#include <RenderMode/ImGuiMode.h>

#include <filesystem>
#include <IBufferManager.h>
#include <SceneAsset/EngineAsset.h>

#define literal_root L"Game\\"

namespace RenderUI {
	namespace fs = std::filesystem;

	using namespace RuntimeAssets;

	struct FileBrowserConfig {
		float Zoom = 1.0f;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(Zoom);
		}
	};

    struct ContentBrowserState {
        std::wstring rootPath;                          // 根目录
        std::wstring currentPath;                       // 当前浏览的目录
        std::string selectedFilePath;                   // 当前选中的文件
    };

	struct FileTypeContext {
		bool IsFolder;
		fs::path entry_path;
	};

	class FileBrowserUI : public SubsystemTemplate<FileBrowserUI, ModeType::ImGui>, public ImGuiMode
	{
	public:
		const ContentBrowserState& GetState() {return state;};

		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;

	protected:
		void DrawLayout();
		void DirTree();
		void File();

		// tools function
		void DrawDirectoryTree(std::wstring path);

		bool IsUnderGameFolder(const fs::path& path);
		void DoubleClickToEnter(FileTypeContext Context);
		void RightClickMenuBar();
		void RightClickMenuBar_File(FileTypeContext Context);

		EngineAssetType GetFileType(const fs::directory_entry& entry);
		ID3D11ShaderResourceView* GetFileIcon(EngineAssetType filetype);

		void FileContentAreaInput(); // 控制文件内容区域输入

		void NewFolder();
		void DeleteFolderOrFile(std::wstring Terget,bool isfile = false);

		// FileOperations
		void FileOperations(FileTypeContext Context);

	public: // ---------------------------- function API
		std::wstring GetAbsolutePath(const std::wstring& currentPath);
		std::wstring ToGameRelative(const std::wstring& absolutePath);
	private:
		ContentBrowserState state;

		float dirTreeWidth = 250.0f;
		const float splitterWidth = 4.0f;

		const float itemSize = 80.0f;        // 每个格子的大小 默认
		const float spacing = 10.0f;         // 格子间距 默认

		// 保存的位置
		ImVec2 DirTreePos;
		ImVec2 FilePos;

		FileBrowserConfig Config;
	};
}