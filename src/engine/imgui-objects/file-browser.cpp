#include <algorithm>
#include <engine/imgui-objects/file-browser.hpp>
#include <imgui.h>
#include <iostream>
namespace Engine {

void FileBrowser::Initialize() {
    // can be used to set directory using top bar like vscode does with a button
    // in the area
}

void FileBrowser::UpdateOnWindowResize(FileBrowserState &state, int w, int h) {
    state.mainWindowWidth = w;
    state.mainWindowHeight = h;
    state.mainwindowresize = true;
    // 40% width, 60% height
    state.filebrowserWidth = 0.4f * static_cast<float>(w);
    state.filebrowserHeight = 0.6f * static_cast<float>(h);
}
bool FileBrowser::SetRootDirectory(FileBrowserState &state,
                                   const std::string &path) {
    try {
        fs::path p(path);
        if (path.empty() || !fs::exists(p) || !fs::is_directory(p))
            return false;
        state.root_directory = fs::canonical(p).string();
        state.has_root = true;
        state.selected_path.clear();
        return true;
    } catch (const fs::filesystem_error &e) {
        std::cerr << "SetRootDirectory error: " << e.what() << std::endl;
        return false;
    }
}

void FileBrowser::OpenFile(const std::string &path) {
    std::cout << "OpenFile: " << path << std::endl;
    // Opening file logic
}

void FileBrowser::RenderDirectoryNode(FileBrowserState &state,
                                      const fs::path &dirPath, int depth) {
    // Collect entries (dirs first, then files)
    std::vector<fs::directory_entry> entries;
    try {
        for (const auto &entry : fs::directory_iterator(dirPath)) {
            entries.push_back(entry);
        }
    } catch (const fs::filesystem_error &e) {
        ImGui::TextDisabled("  <error: %s>", e.what());
        return;
    }

    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry &a, const fs::directory_entry &b) {
                  bool ad = a.is_directory();
                  bool bd = b.is_directory();
                  if (ad != bd)
                      return ad > bd; // dirs first
                  return a.path().filename().string() <
                         b.path().filename().string();
              });

    for (auto &entry : entries) {
        const fs::path &p = entry.path();
        std::string name = p.filename().string();
        bool isDir = entry.is_directory();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
        if (!isDir)
            flags |=
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (state.selected_path == p.string())
            flags |= ImGuiTreeNodeFlags_Selected;

        bool open = false;
        if (isDir) {
            open = ImGui::TreeNodeEx((name + "##" + p.string()).c_str(), flags);
        } else {
            ImGui::TreeNodeEx((name + "##" + p.string()).c_str(), flags);
        }

        if (ImGui::IsItemClicked()) {
            state.selected_path = p.string();
            if (!isDir) {
                OpenFile(p.string());
            }
        }

        if (open && isDir) {
            // Limit depth if desired (optional)
            if (depth < 64) {
                RenderDirectoryNode(state, p, depth + 1);
            } else {
                ImGui::TextDisabled("  <max depth reached>");
            }
            ImGui::TreePop();
        }
    }
}

void FileBrowser::Render(FileBrowserState &state) {
    if (state.mainwindowresize) {
        ImGui::SetNextWindowSize(
            ImVec2(state.filebrowserWidth, state.filebrowserHeight),
            ImGuiCond_Always);
        state.mainwindowresize = false;
    } else {
        ImGui::SetNextWindowSize(
            ImVec2(state.filebrowserWidth, state.filebrowserHeight),
            ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_FirstUseEver);
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(120, 120),
                                        ImVec2(FLT_MAX, FLT_MAX));

    if (ImGui::Begin("File Browser", nullptr, ImGuiWindowFlags_NoCollapse)) {
        if (!state.has_root) {
            ImGui::TextWrapped(
                "No root directory set.\nUse File > Open Root... in the top "
                "bar to pick one.");
        } else {
            ImGui::TextWrapped("Root: %s", state.root_directory.c_str());
            if (!state.selected_path.empty()) {
                ImGui::TextWrapped("Selected: %s", state.selected_path.c_str());
            }
            ImGui::Separator();

            // Draw tree starting at root
            fs::path root(state.root_directory);
            // root is first node
            ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_SpanAvailWidth;
            if (state.selected_path == state.root_directory)
                rootFlags |= ImGuiTreeNodeFlags_Selected;

            std::string rootLabel =
                (root.filename().string().empty() ? root.string()
                                                  : root.filename().string()) +
                std::string("##") + state.root_directory;
            bool rootOpen = ImGui::TreeNodeEx(rootLabel.c_str(), rootFlags);

            if (ImGui::IsItemClicked()) {
                state.selected_path = state.root_directory;
            }

            if (rootOpen) {
                RenderDirectoryNode(state, root, 0);
                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

} // namespace Engine