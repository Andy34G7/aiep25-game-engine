#include <engine/imgui-objects/file-browser.hpp>
#include <imgui.h>
#include <iostream>

namespace Engine {

void FileBrowser::Initialize() {
    // can be used to set directory using top bar
}

void FileBrowser::UpdateOnWindowResize(FileBrowserState& state, int w, int h) {
    state.mainWindowWidth = w;
    state.mainWindowHeight = h;
    state.mainwindowresize = true;

    // Example: keep 40% width, 60% height
    state.filebrowserWidth  = 0.4f * static_cast<float>(w);
    state.filebrowserHeight = 0.6f * static_cast<float>(h);
}

void FileBrowser::PopulateDirectoryContents(FileBrowserState& state) {
    if (state.current_directory.empty()) {
        state.current_directory = fs::current_path().string();
    }
    std::vector<fs::directory_entry> new_contents;
    try {
        for (const auto& entry : fs::directory_iterator(state.current_directory)) {
            new_contents.push_back(entry);
        }
        state.directory_contents = std::move(new_contents);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
        state.directory_contents.clear();
    }
}

void FileBrowser::OpenFile(const std::string& path) {
    std::cout << "OpenFile: " << path << std::endl;
    // Opening file logic
}

void FileBrowser::Render(FileBrowserState& state) {
    // ensure contents are available
    if (state.directory_contents.empty()) {
        PopulateDirectoryContents(state);
    }

    // Size/pos hints (use FirstUseEver so user can resize later)
    if (state.mainwindowresize) {
        ImGui::SetNextWindowSize(ImVec2(state.filebrowserWidth, state.filebrowserHeight), ImGuiCond_Always);
        state.mainwindowresize = false;
    } else {
        ImGui::SetNextWindowSize(ImVec2(state.filebrowserWidth, state.filebrowserHeight), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_FirstUseEver); // below main menu bar
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(50, 50), ImVec2(FLT_MAX, FLT_MAX));

    if (ImGui::Begin("File Browser", nullptr, ImGuiWindowFlags_NoCollapse)) {
        // Navigation row
        if (ImGui::Button("Go Back")) {
            fs::path p = state.current_directory;
            if (p.has_parent_path()) {
                auto parent = p.parent_path();
                if (parent != p) {
                    state.current_directory = parent.string();
                    PopulateDirectoryContents(state);
                }
            }
        }
        ImGui::SameLine();
        ImGui::TextWrapped("Current: %s", state.current_directory.c_str());
        ImGui::Separator();

        if (ImGui::BeginChild("FileListRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar)) {
            std::string selected_dir;
            for (const auto& entry : state.directory_contents) {
                std::string filename = entry.path().filename().string();
                std::string label;

                if (entry.is_directory())      label = "[D] " + filename;
                else if (entry.is_regular_file()) label = "[F] " + filename;
                else                             label = "[?] " + filename;

                label += "##" + entry.path().string(); // unique ID

                if (ImGui::Selectable(label.c_str())) {
                    if (entry.is_directory()) {
                        selected_dir = entry.path().string();
                    } else if (entry.is_regular_file()) {
                        OpenFile(entry.path().string());
                    }
                }
            }

            if (!selected_dir.empty()) {
                try {
                    if (fs::exists(selected_dir) && fs::is_directory(selected_dir)) {
                        state.current_directory = selected_dir;
                        PopulateDirectoryContents(state);
                    } else {
                        PopulateDirectoryContents(state); // refresh
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Error accessing directory: " << e.what() << std::endl;
                    PopulateDirectoryContents(state);
                }
            }

            ImGui::EndChild();
        }
    }
    ImGui::End();
}

} // namespace Engine