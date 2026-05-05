#pragma once
#include <string>
#include <Subsystem/SubsystemTemplate.h>

#include <filesystem>
#include <GuiInterface.h>
#include <IBufferManager.h>
#include <SceneAsset/EngineAsset.h>

#define literal_root L"Game\\"

namespace RenderUI {
	namespace fs = std::filesystem;

	using namespace EngineScene;

    struct ContentBrowserState {
        std::wstring rootPath;                          // 根目录
        std::wstring currentPath;                       // 当前浏览的目录
        std::string selectedFilePath;                   // 当前选中的文件
    };

	struct FileTypeContext {
		bool IsFolder;
		fs::path entry;
	};

	class FileBrowserUI : public SubsystemTemplate<FileBrowserUI,Core::SubsystemContext::Priority::Low>
	{
	public:
		const ContentBrowserState& GetState() {return state;};

		virtual bool Init();
		virtual void Uninstall();


		void Draw();

	protected:
		void DrawLayout();
		void DirTree();
		void File();

		// tools function
		void DrawDirectoryTree(std::wstring path);

		bool IsUnderGameFolder(const fs::path& path);
		void DoubleClickToEnter(FileTypeContext filetype);
		std::wstring GetAbsolutePath(const std::wstring& currentPath);
		std::wstring ToGameRelative(const std::wstring& absolutePath);

		EngineAssetType GetFileType(const fs::directory_entry& entry);
		ID3D11ShaderResourceView* GetFileIcon(EngineAssetType filetype, bool isHovered);
	private:
		ContentBrowserState state;

		float dirTreeWidth = 250.0f;
		const float splitterWidth = 2.0f;

		// 保存的位置
		ImVec2 DirTreePos;
		ImVec2 FilePos;
	};
}