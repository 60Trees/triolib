/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#include <triolib.hpp>

#include <opengl.hpp>
#include <unordered_set>
#include <string_view>
#include <SDL3/SDL.h>

struct Renderer : triolib::AppModule {
    SDL_Window* window = nullptr;
    SDL_GLContext glcontext = nullptr;
    std::unordered_set<std::string_view> gl_extensions{};

    inline bool has_extension(std::string_view ext) { return gl_extensions.contains(ext); }
};
