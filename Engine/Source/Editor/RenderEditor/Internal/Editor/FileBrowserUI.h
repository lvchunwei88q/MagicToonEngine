#pragma once
#include <string>
#include <Object.h>
#include <RenderSubsystem/RenderSubsystem.h>
#include <RenderUIWarehouse.h>

#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>

#include <filesystem>
#include <IBufferManager.h>
#include <AssetPipeline.h>

#include <FileBrowserUI.object.generate.h>

#define literal_root L"Game\\"

using namespace RenderUI;

namespace RenderEditor {
	namespace fs = std::filesystem;

	using namespace AssetPipeline;

	MCLASS(MSERIALIZATION)
	class FileBrowserConfig : Core::Object {
		GENERATE_BODY(FileBrowserConfig,ENGINE,"EDITORUI");
	public:
		MMEMBER();
		float Zoom = 1.0f;
		MMEMBER();
		float dirTreeWidth = 250.0f;
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

	class FileBrowserUI final : public RSubsystemTemplate<FileBrowserUI, ModeType::ImGui>, public ImGuiMode
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

		UIAssetType GetFileType(const fs::directory_entry& entry);
		ID3D11ShaderResourceView* GetFileIcon(UIAssetType filetype);

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
		const float splitterWidth = 4.0f;

		const float itemSize = 80.0f;        // 每个格子的大小 默认
		const float spacing = 10.0f;         // 格子间距 默认

		// 保存的位置
		ImVec2 DirTreePos;
		ImVec2 FilePos;

		std::unique_ptr<FileBrowserConfig> Config;
	};
}