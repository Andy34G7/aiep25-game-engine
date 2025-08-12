#include <SDL3/SDL.h>
#include <cmath>
#include <engine/file-browser.hpp>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

namespace Engine {

void FileBrowser::Initialize() {
    // Provision for any other initialisation
}

void FileBrowser::InitializeImGui(void *window, void *renderer) {
    if (m_imguiInitialized)
        return;

    // Store the renderer for later use
    m_renderer = renderer;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(static_cast<SDL_Window *>(window),
                                      static_cast<SDL_Renderer *>(renderer));
    ImGui_ImplSDLRenderer3_Init(static_cast<SDL_Renderer *>(renderer));

    m_imguiInitialized = true;
}

void FileBrowser::ShutdownImGui() {
    if (!m_imguiInitialized)
        return;

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    m_imguiInitialized = false;
    m_renderer = nullptr;
}

void FileBrowser::ProcessEvent(void *event) {
    if (!m_imguiInitialized)
        return;

    ImGui_ImplSDL3_ProcessEvent(static_cast<SDL_Event *>(event));
}

void FileBrowser::PopulateDirectoryContents(FileBrowserState &state) {
    std::vector<fs::directory_entry> new_contents;
    bool success = true;

    try {
        for (const auto &entry :
             fs::directory_iterator(state.current_directory)) {
            new_contents.push_back(entry);
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
        success = false;
    }

    if (success) {
        state.directory_contents = new_contents;
    } else {
        state.directory_contents.clear();
    }
}

void FileBrowser::OpenFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (file.is_open()) {
        std::cout << "File opened successfully: " << filePath << std::endl;
        // File opening logic
        file.close();
    } else {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }
}

void FileBrowser::RenderImGuiFrame(FileBrowserState &state) {
    // Populate directory contents if empty (first time or after error)
    if (state.directory_contents.empty()) {
        PopulateDirectoryContents(state);
    }

    // Set window size constraints BEFORE Begin()
    ImGui::SetNextWindowSizeConstraints(ImVec2(200, 150),
                                        ImVec2(FLT_MAX, FLT_MAX));

    // Initialize the window
    bool filebrowser_isopen =
        ImGui::Begin("File Browser", nullptr, ImGuiWindowFlags_NoCollapse);

    // Check for resizing
    if (state.main_window_resized) {
        ImGui::SetWindowSize(ImVec2(state.width, state.height),
                             ImGuiCond_Always);
        state.main_window_resized = false;
    } else {
        ImGui::SetWindowSize(ImVec2(state.width, state.height),
                             ImGuiCond_FirstUseEver);
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    }

    // Check for manual sizing
    if (filebrowser_isopen) {
        ImVec2 currentSize = ImGui::GetWindowSize();

        const float threshold = 1.0f;
        if (std::fabs(currentSize.x - state.width) > threshold ||
            std::fabs(currentSize.y - state.height) > threshold) {

            state.width = currentSize.x;
            state.height = currentSize.y;

            // Update ratio
            state.ratio_width =
                currentSize.x / static_cast<float>(state.main_window_width);
            state.ratio_height =
                currentSize.y / static_cast<float>(state.main_window_height);
        }

        // UI elements

        // Go Back button
        if (ImGui::Button("Go Back")) {
            fs::path current_path = state.current_directory;
            if (current_path.has_parent_path()) {
                // Check if parent path is different before navigating
                fs::path parent_path = current_path.parent_path();
                if (parent_path !=
                    current_path) { // Avoid getting stuck at root
                    state.current_directory = parent_path.string();
                    PopulateDirectoryContents(state);
                }
            }
        }
        ImGui::SameLine();
        ImGui::TextWrapped(
            "Current: %s",
            state.current_directory.c_str()); // Use TextWrapped for long paths
        ImGui::Separator();                   // Visual separation

        // Child Object to make listbox occupy the whole window
        if (ImGui::BeginChild("FileListRegion", ImVec2(0, 0), true,
                              ImGuiWindowFlags_HorizontalScrollbar)) {

            std::string selected_directory_path; // Capture path

            for (const auto &entry : state.directory_contents) {
                std::string filename = entry.path().filename().string();
                std::string label;

                if (entry.is_directory()) {
                    label = "[D] " + filename; // Simple indicator for directory
                } else if (entry.is_regular_file()) {
                    label = "[F] " + filename; // Simple indicator for file
                } else {
                    label = "[?] " + filename; // Unknown type
                }
                label += "##" + entry.path().string();

                if (ImGui::Selectable(label.c_str())) {
                    if (entry.is_directory()) {
                        selected_directory_path =
                            entry.path().string(); // Store the path for
                                                   // processing after loop
                    } else if (entry.is_regular_file()) {
                        std::cout << "Opening file: " << entry.path()
                                  << std::endl;
                        OpenFile(entry.path().string());
                    }
                }
            }

            // Process the selected directory after the loop to avoid modifying
            // the list while iterating
            if (!selected_directory_path.empty()) {
                try {
                    if (fs::exists(selected_directory_path) &&
                        fs::is_directory(selected_directory_path)) {
                        state.current_directory = selected_directory_path;
                        PopulateDirectoryContents(state);
                    } else {
                        std::cerr << "Selected path no longer exists or is not "
                                     "a directory: "
                                  << selected_directory_path << std::endl;
                        PopulateDirectoryContents(state);
                    }
                } catch (const fs::filesystem_error &e) {
                    std::cerr << "Error accessing directory: " << e.what()
                              << std::endl;
                    PopulateDirectoryContents(state);
                }
            }
        }
        ImGui::EndChild(); // End File List
    }
    ImGui::End(); // End File Browser window
}

void FileBrowser::Render(FileBrowserState &state) {
    if (!m_imguiInitialized) {
        std::cerr << "ImGui not initialized! Call InitializeImGui first."
                  << std::endl;
        return;
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    RenderImGuiFrame(state);

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(
        ImGui::GetDrawData(), static_cast<SDL_Renderer *>(m_renderer));
}

void FileBrowser::UpdateOnWindowResize(FileBrowserState &state, int newWidth,
                                       int newHeight) {
    state.main_window_width = newWidth;
    state.main_window_height = newHeight;

    // Update file browser size based on new window size
    state.width = static_cast<float>(newWidth) * state.ratio_width;
    state.height = static_cast<float>(newHeight) * state.ratio_height;

    // Add minimum size conditions
    if (state.width < 200.0f)
        state.width = 200.0f;
    if (state.height < 150.0f)
        state.height = 150.0f;

    // Update flag
    state.main_window_resized = true;
}

} // namespace Engine
