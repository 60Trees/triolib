/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#include <base/fps_handler.hpp>

#include <SDL3/SDL.h>
#include <cmath>

using namespace std;
using namespace triolib;

inline double normalize(const double input) {
    if (isnan(input)) return 1.0;
    if (isinf(input)) return 1.0;
    if (input < 0) return -input;
    return input;
}

struct FpsHandlerImpl : FpsHandler {
    std::vector<SpeedMultiplier*> slomos;

    void init() override { slomos = parent->get_mods<SpeedMultiplier>(); }

    void loop() override {
        const auto newticks = SDL_GetTicks();
        ticksElapsed = newticks - ticks;
        ticks = newticks;
        dt = (double)(ticksElapsed) / 1000.0;
        for (auto* mod : slomos) dt *= normalize(mod->getmultiplier());
    }

    void quit() override {}
} REGISTER_MOD(FpsHandlerImpl);
