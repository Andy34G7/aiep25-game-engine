#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace Engine {

struct FileBrowserState {
    std::string root_directory;
    bool has_root = false;
    std::string selected_path;

    // sizing state (percentages tracked elsewhere if needed)
    int mainWindowWidth = 1000;
    int mainWindowHeight = 800;
    float filebrowserWidth = 400.f;
    float filebrowserHeight = 480.f;
    bool mainwindowresize = false;
};

class FileBrowser {
  public:
    void Initialize();
    void UpdateOnWindowResize(FileBrowserState &state, int w, int h);
    bool SetRootDirectory(FileBrowserState &state, const std::string &path);

    void Render(FileBrowserState &state);

  private:
    void RenderDirectoryNode(FileBrowserState &state, const fs::path &dirPath,
                             int depth = 0);
    void OpenFile(const std::string &path);
};

} // namespace Engine