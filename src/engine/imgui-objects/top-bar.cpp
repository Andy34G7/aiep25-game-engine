#include <engine/imgui-objects/top-bar.hpp>
#include <imgui.h>

namespace Engine {
void TopBar::Render() {
    if (ImGui::BeginMainMenuBar()) {
        // Add menu items
        ImGui::EndMainMenuBar();
    }
}
} // namespace Engine