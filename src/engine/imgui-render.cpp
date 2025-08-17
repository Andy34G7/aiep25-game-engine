#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <engine/imgui-render.hpp>
#include <imgui.h>

namespace Engine {
void ImGuiRender::Init(SDL_Window *window, SDL_Renderer *renderer) {
    if (m_initialized)
        return;
    m_window = window;
    m_renderer = renderer;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
    ImGui_ImplSDLRenderer3_Init(m_renderer);

    m_initialized = true;
}

void ImGuiRender::ShutDown() {
    if (!m_initialized)
        return;
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    m_window = nullptr;
    m_renderer = nullptr;
    m_initialized = false;
}

void ImGuiRender::EndFrame() {
    if (!m_initialized)
        return;
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
}

void ImGuiRender::ProcessEvent(const SDL_Event *e) {
    if (!m_initialized || !e)
        return;
    ImGui_ImplSDL3_ProcessEvent(e);
}
} // namespace Engine