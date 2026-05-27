#pragma once
#include <RenderSubsystem/RenderSubsystem.h>
#include "AssetStruct.h"
#include <RenderUIWarehouse.h>
#include <cereal/cereal.hpp>
#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>
#include <Theme.h>

using namespace Theme;
using namespace RenderUI;

namespace RenderLauncher {

	enum class UIType : uint8_t
	{
		Launcher,
		Loading,
	};

	enum class LauncherUIType : uint8_t
	{
		NewProject,
		OpenProject,

		Exit,
	};

	class MenuButton {
	public:
		MenuButton(std::string label)
			: m_label(label) {
		}

		bool Draw(const ImVec2& size, bool is_selected) {
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

			// 根据选中状态设置不同颜色方案
			if (is_selected) {
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(ACCENT_R, ACCENT_G, ACCENT_B, 0.8f));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ACCENT_R, ACCENT_G, ACCENT_B, 0.25f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ACCENT_R, ACCENT_G, ACCENT_B, 0.40f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ACCENT_R, ACCENT_G, ACCENT_B, 0.60f));
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.4f, 0.4f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.8f));
			}

			bool clicked = ImGui::Button(m_label.c_str(), size);
			ImGui::PopStyleColor(4);
			ImGui::PopStyleVar(2);

			return clicked;
		}

	private:
		std::string m_label;
	};

	class LauncherGeneralLayout final : public RSubsystemTemplate<LauncherGeneralLayout, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;

		virtual void* PublicData(uint8_t type) override;

		[[nodiscard]] virtual int GetPriority() const override {
			return -100; // High
		}

		void SetUIType(UIType newType) {
			uiType = newType;
		}

	private:
		size_t ComputeJSONHash(const JSON& data);
		size_t ComputeStringHash(const std::string& data);

		bool LoadProjectList();

		// UI Function
		void DrawNewProjectUI();
		void DrawOpenProjectUI();

		void OpenProject(std::string path, std::string name);
		void ExitProgram();

		bool m_bDragging = false;
		ImVec2 m_dragOffset;

		LauncherUIType Type = LauncherUIType::NewProject;
		UIType uiType = UIType::Launcher;

		MenuButton* new_btn  = nullptr;
		MenuButton* open_btn = nullptr;
		MenuButton* exit_btn = nullptr;

		std::vector<ProjectInfo> projects;

		friend void Launcher(LauncherGeneralLayout* This);
		friend void Loading(LauncherGeneralLayout* This);
	};
}