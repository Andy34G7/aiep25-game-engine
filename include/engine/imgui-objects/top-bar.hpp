#pragma once
#include <engine/imgui-objects/file-browser.hpp>
#include <string>

namespace Engine {
struct TopBar {
    static void Render(FileBrowser &browser, FileBrowserState &state);
};
} // namespace Engine