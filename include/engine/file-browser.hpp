#pragma once

#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace Engine {

struct FileBrowserState {
    std::string current_directory = fs::current_path().string();
    std::vector<fs::directory_entry> directory_contents;

    // File browser dimensions and ratios
    float width = 400.0f;
    float height = 600.0f;
    float ratio_width = 0.4f;
    float ratio_height = 0.6f;

    // Window state
    bool main_window_resized = false;
    int main_window_width = 1000;
    int main_window_height = 800;
};

class FileBrowser {
  public:
    FileBrowser() = default;
    ~FileBrowser() = default;

    void Initialize();
    void Render(FileBrowserState &state);
    void UpdateOnWindowResize(FileBrowserState &state, int newWidth,
                              int newHeight);
    void InitializeImGui(void *window, void *renderer);
    void ShutdownImGui();
    void ProcessEvent(void *event); // Add this method

  private:
    void PopulateDirectoryContents(FileBrowserState &state);
    void OpenFile(const std::string &filePath);
    void RenderImGuiFrame(FileBrowserState &state);

    bool m_imguiInitialized = false;
    void *m_renderer = nullptr;
};

} // namespace Engine