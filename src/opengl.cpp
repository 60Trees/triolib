// <AI>
#include <SDL3/SDL.h>

#include <opengl.hpp>
#include <iostream>

void set_opengl_version() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_VERSION_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_VERSION_MINOR);

    std::cout << "[OPENGL] Requesting OpenGL " << GL_VERSION_MAJOR << "." << GL_VERSION_MINOR << std::endl;

#ifndef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#    ifdef DO_GL_DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#    endif
#endif
}
// </AI>
