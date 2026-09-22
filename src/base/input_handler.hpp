/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#include <triolib.hpp>
#include <SDL3/SDL_events.h>

struct InputHandler : triolib::AppModule {
    virtual void digest_event(const SDL_Event&) = 0;
    void init() override {}
    void loop() override {}
    void quit() override {}
};

