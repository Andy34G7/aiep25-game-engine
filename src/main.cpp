#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h> 
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui_internal.h> // for docking imgui stuff
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <fstream> // For file handling


namespace fs = std::filesystem;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;


const int BG_COLOR_RED = 255; //added "BG" 
const int BG_COLOR_GREEN = 0; //to avoid confusion
const int BG_COLOR_BLUE = 0;
const int BG_COLOR_ALPHA = 128;


const SDL_Color TEXTBOX_BG_COLOR = {255, 255, 255, 255}; // White, Opaque
const SDL_Color TEXT_COLOR = {0, 0, 0, 255};             // Black, Opaque

const char* FONT_PATH = "../src/Montserrat-VariableFont_wght.ttf"; //Not Arial :P
const int FONT_SIZE = 34;
const char* TEXTBOX_TEXT = "Game Engine";
const float PADDING = 15.0f;

struct AppState {
    SDL_Window *mainWindow;
    SDL_Renderer *mainRenderer;
    TTF_Font *font = nullptr;
    SDL_Texture *textTexture = nullptr;
    SDL_FRect textBoxRect = {0, 0, 0, 0};
    SDL_FRect textRect = {0, 0, 0, 0};
    SDL_AppResult appResult = SDL_APP_CONTINUE;

    // File Browser Data
    std::string current_directory = fs::current_path().string();
    std::vector<fs::directory_entry> directory_contents;

    // Main window dimensions
    int mainWindowWidth = SCREEN_WIDTH;
    int mainWindowHeight = SCREEN_HEIGHT;

    //File browser dimensions
    float filebrowserWidth = SCREEN_WIDTH*0.4f;
    float filebrowserHeight = SCREEN_HEIGHT*0.6f;
    bool mainwindowresize = false;
    float filebrowser_ratio_width = 0.4f;
    float filebrowser_ratio_height = 0.6f;
};

// A simple helper function that logs that a critical failure happens and
// returns an app failure.
SDL_AppResult SDL_Failure(const char *fmt) {
    SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", fmt, SDL_GetError());
    // Check ttfError
    // const char* ttfError = TTF_GetError();
    // if (ttfError[0] != '\0') { // Returns empty string if no error
    //      SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "TTF Error: %s\n", ttfError);
    // }
    return SDL_APP_FAILURE;
}

// For Opening file
void OpenFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (file.is_open()) {
        std::cout << "File opened successfully: " << filePath << std::endl;
        // Add the necessary stuff
        file.close();
    } else {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }
}

void populate_directory_contents(AppState* state) {
    std::vector<fs::directory_entry> new_contents;
    bool success = true;
    try {
        for (const auto& entry : fs::directory_iterator(state->current_directory)) {
            new_contents.push_back(entry);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
        success = false;
    }

    if (success) {
        state->directory_contents = new_contents;
    } else {
        state->directory_contents.clear(); // Ensure it's clear on failure
    }
}

void FileBrowserUI(AppState* state) {
    
    // Initialise the window
    bool filebrowser_isopen = ImGui::Begin("File Browser", nullptr, ImGuiWindowFlags_NoCollapse);
    
    //Check for resizing
    if(state->mainwindowresize) {
        ImGui::SetWindowSize(ImVec2(state->filebrowserWidth, state->filebrowserHeight), ImGuiCond_Always);
        state->mainwindowresize = false;
    }
    else {
        ImGui::SetWindowSize(ImVec2(state->filebrowserWidth, state->filebrowserHeight), ImGuiCond_FirstUseEver);
        ImGui::SetWindowPos(ImVec2(0,0), ImGuiCond_FirstUseEver);
    }
    
    //Set window size constraints
    ImGui::SetNextWindowSizeConstraints(ImVec2(50, 50), ImVec2(FLT_MAX, FLT_MAX));

    //check for manual sizing
    if(filebrowser_isopen){
        ImVec2 currentSize = ImGui::GetWindowSize();
        bool user_resize = false;
        
        //check beyond very small change
        const float threshold = 0.1f;
        if (fabs(currentSize.x - state->filebrowserWidth) > threshold ||
            fabs(currentSize.y - state->filebrowserHeight) > threshold) {
            user_resize = true;
            state->filebrowserWidth = currentSize.x;
            state->filebrowserHeight = currentSize.y;
            //Update ratio
            state->filebrowser_ratio_width = currentSize.x / (float)state->mainWindowWidth;
            state->filebrowser_ratio_height = currentSize.y / (float)state->mainWindowHeight;
        }

    }

    // UI elements

        //Go Back button
        if (ImGui::Button("Go Back")) {
            fs::path current_path = state->current_directory;
            if (current_path.has_parent_path()) {
                // Check if parent path is different before navigating
                fs::path parent_path = current_path.parent_path();
                if (parent_path != current_path) { // Avoid getting stuck at root
                     state->current_directory = parent_path.string();
                     populate_directory_contents(state);
                }
            }
        }
        ImGui::SameLine(); 
        ImGui::TextWrapped("Current: %s", state->current_directory.c_str()); // Use TextWrapped for long paths
        ImGui::Separator(); // to make visual separation

        // Child Object to make listbox occupy the whole window
        if (ImGui::BeginChild("FileListRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar)) { // Use 0,0 to fill space, add border, scrollbar
             
            std::string selected_directory_path; // Capture path

             for (const auto& entry : state->directory_contents) {
                  std::string filename = entry.path().filename().string();
                  std::string label; // Construct label with icon (optional)

                  if (entry.is_directory()) {
                       label = "[D] " + filename; // Simple indicator for directory
                  } else if (entry.is_regular_file()) {
                       label = "[F] " + filename; // Simple indicator for file
                  } else {
                       label = "[?] " + filename; // Unknown type
                  }
                  label += "##" + entry.path().string(); // Unique ID for ImGui using full path

                  if (ImGui::Selectable(label.c_str())) {
                       if (entry.is_directory()) {
                            selected_directory_path = entry.path().string(); // Store the path for processing after loop
                       } else if (entry.is_regular_file()) {
                            std::cout << "Opening file: " << entry.path() << std::endl;
                            OpenFile(entry.path().string());
                       }
                  }
             }
             // Process the selected directory after the loop to avoid modifying the list while iterating
             if (!selected_directory_path.empty()) {
                  try {
                       // Check to avoid issues if directory was deleted or moved
                       if (fs::exists(selected_directory_path) && fs::is_directory(selected_directory_path)) {
                            state->current_directory = selected_directory_path;
                            populate_directory_contents(state);
                       } else {
                            std::cerr << "Selected path no longer exists or is not a directory: " << selected_directory_path << std::endl;
                            //re-populate current directory contents to refresh view
                            populate_directory_contents(state);
                       }
                  } catch (const fs::filesystem_error& e) {
                       std::cerr << "Error accessing directory: " << e.what() << std::endl;
                       // Attempt to revert or handle error gracefully
                       // Maybe revert to parent? Or just log error.
                       // For now, just log and potentially refresh
                       populate_directory_contents(state);
                  }
             }
        }
        ImGui::EndChild(); // End FileListRegion
    ImGui::End(); // End File Browser window
}

// Initialises subsystems and initialises appState to be used by all other main
// functions.
SDL_AppResult SDL_AppInit(void** appState, int argc, char* argv[]) {
    AppState* state = new AppState();
    state->mainWindow = nullptr;
    state->mainRenderer = nullptr; // This only is useful this way if we are initialising
                       // subsystems using pointers, which isn't very common. We
                       // can instead initialise the state after all subsystems
                       // are initialised, but this depends on which subsystems
                       // we are initialising.
    SDL_SetAppMetadata("GameEngine", "0.0.1",
                       "org.acm.pesuecc.aiep.game-engine");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return SDL_Failure("Error initialising SDL3");
    }

    //SDL_ttf
    if (!TTF_Init()) {
        SDL_Quit();
        return SDL_Failure("Error initialising SDL_ttf");
    }

    bool initWindowRenderer{SDL_CreateWindowAndRenderer(
        "GameEngine", SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_RESIZABLE , &(state->mainWindow),
        &(state->mainRenderer))};
    if (!initWindowRenderer) {
        return SDL_Failure("Error creating Window and Renderer");
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(state->mainWindow, state->mainRenderer);
    ImGui_ImplSDLRenderer3_Init(state->mainRenderer);

    
    state->font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
    if (!state->font) { //if font fails to load
        SDL_DestroyRenderer(state->mainRenderer);
        SDL_DestroyWindow(state->mainWindow);
        TTF_Quit();
        SDL_Quit();
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to load font '%s'", FONT_PATH);
        return SDL_Failure("Font loading error");
    }
    SDL_Surface *textSurface = TTF_RenderText_Blended(state->font, TEXTBOX_TEXT, 32, TEXT_COLOR); //render text onto surface
    //Point to self: Blended is nicer
    if (!textSurface) { //text surface error handling
        TTF_CloseFont(state->font);
        SDL_DestroyRenderer(state->mainRenderer);
        SDL_DestroyWindow(state->mainWindow);
        TTF_Quit();
        SDL_Quit();
        delete state;
        return SDL_Failure("Failed to render text surface");
    }

    //Making a texture using the surface
    state->textTexture = SDL_CreateTextureFromSurface(state->mainRenderer, textSurface);
    if (!state->textTexture) {
        SDL_DestroySurface(textSurface);
        TTF_CloseFont(state->font);
        SDL_DestroyRenderer(state->mainRenderer);
        SDL_DestroyWindow(state->mainWindow);
        TTF_Quit();
        SDL_Quit();
        delete state;
        return SDL_Failure("Failed to create text texture");
    }

    float textW = textSurface->w; //width,
    float textH = textSurface->h; //height of the text surface

    SDL_DestroySurface(textSurface);

    //float padding = 15.0f; //Padding around text cuz nice
    state->textBoxRect.w = textW + 2 * PADDING;
    state->textBoxRect.h = textH + 2 * PADDING;

    state->textBoxRect.x = (SCREEN_WIDTH - state->textBoxRect.w) / 2.0f; //putting the textbox in the 
    state->textBoxRect.y = (SCREEN_HEIGHT - state->textBoxRect.h) / 2.0f; //center of the screen

    state->textRect.x = state->textBoxRect.x + PADDING; //putting the text
    state->textRect.y = state->textBoxRect.y + PADDING; //in the center of the textbox
    state->textRect.w = textW;                          //cuz why not
    state->textRect.h = textH;

    populate_directory_contents(state); // Initial population of file browser

    *appState = state;
    return SDL_APP_CONTINUE;
}

// The event handling function called by SDL.
// TODO: Try and make a genericsed function that is called from here to allow
//       custom event handling to be added by a user.
SDL_AppResult SDL_AppEvent(void *appState, SDL_Event *event) {
    AppState *state = static_cast<AppState *>(appState);
    ImGui_ImplSDL3_ProcessEvent(event); // Pass SDL events to ImGui

    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_TERMINATING) {
        state->appResult = SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        SDL_Log("Window resized to %d x %d", event->window.data1, event->window.data2);
        int newWidth, newHeight;
        SDL_GetWindowSize(state->mainWindow, &newWidth, &newHeight);

        // Store the new dimensions in the AppState
        state->textBoxRect.x = (newWidth - state->textBoxRect.w) / 2.0f;
        state->textBoxRect.y = (newHeight - state->textBoxRect.h) / 2.0f;
        state->textRect.x = state->textBoxRect.x + PADDING;
        state->textRect.y = state->textBoxRect.y + PADDING;

        state->mainWindowWidth = newWidth;  // Add this to AppState
        state->mainWindowHeight = newHeight; // Add this to AppState

        // Update file broswer size based on new window size
        state->filebrowserWidth = (float)state->mainWindowWidth * state->filebrowser_ratio_width;
        state->filebrowserHeight = (float)state->mainWindowHeight * state->filebrowser_ratio_height;

        //add minimum size conditions
        if (state->filebrowserWidth < 50.0f) state->filebrowserWidth = 50.0f;
        if (state->filebrowserHeight < 50.0f) state->filebrowserHeight = 50.0f;

        // Update flag
        state->mainwindowresize = true;
    }
    else if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_Log("Key pressed: %c", event->key.key);
    }
    else if (event->type == SDL_EVENT_KEY_UP) {
        SDL_Log("Key was released: %c", event->key.key);
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        SDL_Log("Mouse button pressed");
    }
    else if (event->type == SDL_EVENT_MOUSE_WHEEL) {
        SDL_Log("Mouse wheel scrolled: %.0f", event->wheel.y);
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        SDL_Log("Mouse button released");
    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION) {
        SDL_Log("Mouse moved to: (%.0f, %.0f)", event->motion.x, event->motion.y);
    }

    return SDL_APP_CONTINUE;
}

// The "main loop" of the window.
SDL_AppResult SDL_AppIterate(void *appState) {
    AppState *state = static_cast<AppState *>(appState);

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // ImGui UI
    FileBrowserUI(state); // Render the file browser

    // ImGui::Begin("Game Engine Info"); // Keep your original ImGui window
    // ImGui::Text("Hello from ImGui!");
    // ImGui::End();

    // Rendering
    ImGui::Render();
    SDL_SetRenderDrawColor(state->mainRenderer, BG_COLOR_RED, BG_COLOR_GREEN,
                           BG_COLOR_BLUE, BG_COLOR_ALPHA);
    SDL_RenderClear(state->mainRenderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), state->mainRenderer);
        // draw bg rect
    SDL_SetRenderDrawColor(state->mainRenderer, TEXTBOX_BG_COLOR.r, TEXTBOX_BG_COLOR.g,
            TEXTBOX_BG_COLOR.b, TEXTBOX_BG_COLOR.a);
    SDL_RenderFillRect(state->mainRenderer, &(state->textBoxRect));
    // drawing the text texture
    // The source rect is NULL to draw the entire texture.
    // The destination rect is where on the screen to draw it.
    SDL_RenderTexture(state->mainRenderer, state->textTexture, nullptr, &(state->textRect));
    SDL_RenderPresent(state->mainRenderer);
    return state->appResult;
}

// Cleans up the initialised subsystems.
void SDL_AppQuit(void *appState, SDL_AppResult result) {
    AppState *state = static_cast<AppState *>(appState);
    if (state != nullptr) {
        // ImGui cleanup
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        // Destroy SDL resources
        SDL_DestroyTexture(state->textTexture);
        TTF_CloseFont(state->font);
        SDL_DestroyRenderer(state->mainRenderer);
        SDL_DestroyWindow(state->mainWindow);
        delete state;
    }
}
