/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#include <SDL3/SDL_main.h>
#include <opengl.hpp>
#include <triolib.hpp>
#include <iostream>
#include <cmrc/cmrc.hpp>
#ifdef __EMSCRIPTEN__
#    include <emscripten.h>
#endif

CMRC_DECLARE(assets);

std::string_view load_asset(const std::string& path) {
    static auto fs = cmrc::assets::get_filesystem();
    auto file = fs.open(path);
    return {file.begin(), file.size()};
}

#define try_smth(task, taskname, onerr)                                                                        \
    try {                                                                                                      \
        task;                                                                                                  \
    } catch (std::exception & e) {                                                                             \
        std::cerr << "[MAIN] Error " << typeid(e).name() << " while " << taskname << ": " << e.what() << '\n'; \
        {                                                                                                      \
            onerr;                                                                                             \
        }                                                                                                      \
        throw;                                                                                                 \
    }

bool before_main = true;

int main(int argc, char** argv) {
    before_main = false;

    using namespace triolib;
    using namespace std;

    unique_ptr<Application> app = nullptr;

    auto loop = [&] {
#ifdef __EMSCRIPTEN__
        // <AI>
        emscripten_set_main_loop_arg(
            [](void* userdata) {
                auto* app_ptr = static_cast<Application*>(userdata);
                if (app_ptr->done) {
                    emscripten_cancel_main_loop();
                    return;
                }
                app_ptr->loop();
            },
            app.get(), 0 /* let the browser drive fps via rAF */, 1 /* simulate infinite loop */
        );
        // </AI>
#else
        while (!app->done) app->loop();
#endif
    };

    auto deconstruct = [&] {
        cout << "[MAIN] Deconstructing\n";
        try_smth(app = nullptr, "deconstructing", {});
    };

    auto quit = [&] {
        cout << "[MAIN] Quitting\n";
        try_smth(app->quit(), "quitting", { deconstruct(); });
        deconstruct();
    };

    cout << "[MAIN] Constructing\n";
    try_smth(app = create_application(), "constructing", { quit(); });

    try_smth(app->add_all_mods(), "adding mods", { quit(); });

    cout << "[MAIN] Initializing\n";
    try_smth(app->init(), "initializing", { quit(); });

    cout << "[MAIN] Looping\n";
    try_smth(loop(), "looping", { quit(); });

    cout << "[MAIN] Goodbye (:\n";
    return 0;
}
