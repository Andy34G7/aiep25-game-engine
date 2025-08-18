#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace Engine {

struct FileBrowserState {
    std::string current_directory;
    std::vector<fs::directory_entry> directory_contents;

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
    void Render(FileBrowserState &state);

  private:
    void PopulateDirectoryContents(FileBrowserState &state);
    void OpenFile(const std::string &path);
};

} // namespace Engine