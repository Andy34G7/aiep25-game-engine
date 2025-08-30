#include <engine/imgui-objects/top-bar.hpp>
#include <imgui.h>
#include <filesystem>

namespace Engine {
void TopBar::Render(FileBrowser &browser, FileBrowserState &state) {
    static bool openRootPopup = false;
    static char pathBuffer[1024] = {0};
    static bool pathError = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Root...")) {
                openRootPopup = true;
                pathError = false;
                if (state.has_root)
                    strncpy(pathBuffer, state.root_directory.c_str(),
                            sizeof(pathBuffer) - 1);
                else
                    pathBuffer[0] = '\0';
            }
            if (state.has_root) {
                if (ImGui::MenuItem("Clear Root")) {
                    state.has_root = false;
                    state.root_directory.clear();
                    state.selected_path.clear();
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (openRootPopup) {
        ImGui::OpenPopup("Set Root Directory");
        openRootPopup = false;
    }

    if (ImGui::BeginPopupModal("Set Root Directory", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Path", pathBuffer, sizeof(pathBuffer));
        if (pathError) {
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1),
                               "Invalid directory.");
        }
        if (ImGui::Button("Set", ImVec2(80, 0))) {
            std::string candidate(pathBuffer);
            if (browser.SetRootDirectory(state, candidate)) {
                ImGui::CloseCurrentPopup();
            } else {
                pathError = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

}  // namespace Engine