#pragma once
#include <SDL3/SDL.h>

namespace Engine {

    class ImGuiRender {
        public:
            static ImGuiRender& Instance();

            void Init(SDL_Window * window, SDL_Renderer * renderer);
            void ShutDown();

            void ProcessEvent(const SDL_Event* e);

            void BeginFrame();
            void EndFrame();

        
        private:
            SDL_Window*   m_window   = nullptr;
            SDL_Renderer* m_renderer = nullptr;
            bool m_initialized = false;
    };
} // namespace Engine