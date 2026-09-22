/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#include <triolib.hpp>

#include <base/renderer.hpp>
#include <base/input_handler.hpp>

using namespace triolib;

struct Game : Application {
    Renderer& r = get<Renderer>();
    std::vector<InputHandler*> input_handlers{};

    Game() { name = "My Game"; }

    void init() override {
        Application::init();
        input_handlers = get_mods<InputHandler>();
    }

    void loop() override {
        Application::loop();

        for (SDL_Event event; SDL_PollEvent(&event);) {
            if (event.type == SDL_EVENT_QUIT) done = true;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) done = true;

            for (auto* mod : input_handlers) mod->digest_event(event);
        }
    }
} REGISTER_APP(Game);
