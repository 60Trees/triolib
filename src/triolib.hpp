/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#pragma once

#include <memory>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>
#include <utility>
#include <algorithm>

extern bool before_main;

std::string_view load_asset(const std::string& path);

enum BuildType : unsigned char {
    Debug = BUILD_TYPE_DEBUG,
    RelWithDebInfo = BUILD_TYPE_RELWITHDEBINFO,
    MinSizeRel = BUILD_TYPE_MINSIZEREL,
    Release = BUILD_TYPE_RELEASE,
};

constexpr static BuildType build_type = (BuildType)BUILD_TYPE;

namespace triolib {
    struct BaseClass {
        virtual ~BaseClass() = default;

        virtual void init() = 0;
        virtual void loop() = 0;
        virtual void quit() = 0;
    };

    struct Application;

    /// @note When constructing, parent is nullptr!
    struct AppModule : BaseClass {
        Application* parent = nullptr;

        virtual ~AppModule() = default;

        virtual bool does_go_after(const AppModule& other) const { return false; }

        virtual bool does_go_before(const AppModule& other) const { return false; }
    };

    std::vector<std::unique_ptr<AppModule>> get_app_mods();

    struct Application : BaseClass {
        void init() override {
            rebuild_mod_order();

            for (auto* mod : mod_order) mod->init();
        }

        void loop() override {
            for (auto* mod : mod_order) mod->loop();
        }

        void quit() override {
            for (auto* mod : mod_order) mod->quit();
        }

        std::string name{};
        bool done = false;

        std::vector<std::unique_ptr<AppModule>> mods{};
        bool added_all_mods = false;

        private:
        std::vector<AppModule*> mod_order{};

        void rebuild_mod_order() {
            add_all_mods();
            mod_order.clear();
            mod_order.reserve(mods.size());

            for (auto& mod : mods) mod_order.push_back(mod.get());

            for (std::size_t i = 1; i < mod_order.size(); ++i) {
                AppModule* current = mod_order[i];

                std::size_t position = i;

                for (std::size_t j = 0; j < i; ++j) {
                    AppModule* other = mod_order[j];

                    const bool current_after = current->does_go_after(*other);

                    const bool current_before = current->does_go_before(*other);

                    const bool other_after = other->does_go_after(*current);

                    const bool other_before = other->does_go_before(*current);

                    if (current_before || other_after) {
                        position = j;
                        break;
                    }
                    if (current_after || other_before) continue;
                }

                if (position != i) {
                    mod_order.erase(mod_order.begin() + i);
                    mod_order.insert(mod_order.begin() + position, current);
                }
            }
        }

        public:
        template <typename T, typename Factory = std::nullptr_t>
        void depend(Factory factory = nullptr, bool exact = false) {
            add_all_mods();
            static_assert(std::is_base_of_v<AppModule, T>, "T must derive from AppModule");

            for (const auto& _mod : mods) {
                auto* mod = _mod.get();

                if (exact && typeid(*mod) == typeid(T)) return;
                if (!exact && dynamic_cast<T*>(mod)) return;
            }

            std::unique_ptr<T> mod;

            if constexpr (std::is_same_v<Factory, std::nullptr_t>) {
                mod = std::make_unique<T>();
            } else {
                mod = factory();
            }

            mod->parent = this;
            mods.emplace_back(std::move(mod));

            rebuild_mod_order();
        }
        template <typename T>
        std::vector<T*> get_mods() {
            add_all_mods();
            static_assert(std::is_base_of_v<AppModule, T>, "T must derive from AppModule");

            std::vector<T*> result;

            for (auto& _mod : mods) {
                if (auto* mod = dynamic_cast<T*>(_mod.get())) {
                    result.push_back(mod);
                }
            }

            return result;
        }

        void add_all_mods() {
            if (added_all_mods) return;
            if (before_main) throw std::logic_error("Cannot add mods before main!");
            std::cout << "[APPLICATION] Initializing all mods\n";
            auto newmods = get_app_mods();
            for (auto& newmod : newmods) {
                newmod->parent = this;
                mods.push_back(std::move(newmod));
            }
            added_all_mods = true;
        }

        template <typename T>
        T& find_mod() {
            add_all_mods();
            for (auto& mod : mods) {
                T* base_mod = dynamic_cast<T*>(mod);
                if (base_mod) return *base_mod;
            }
            throw std::logic_error("Application module does not exist: " + std::string(typeid(T).name()));
        }

        template <typename T, typename Factory = std::nullptr_t>
        T& depend_get(Factory factory = nullptr, bool exact = false) {
            add_all_mods();
            depend<T>(factory, exact);
            return get<T>(exact);
        }

        template <typename T>
        T& get(bool exact = false) {
            add_all_mods();
            static_assert(std::is_base_of_v<AppModule, T>, "T must derive from AppModule");

            for (auto& _mod : mods) {
                auto* mod = _mod.get();

                if (exact && typeid(*mod) == typeid(T)) return *static_cast<T*>(mod);
                if (!exact && dynamic_cast<T*>(mod)) return *dynamic_cast<T*>(mod);
            }

            throw std::logic_error("Application module does not exist: " + std::string(typeid(T).name()));
        }
        template <typename T>
        inline T& operator()() {
            return get<T>(false);
        }
    };
    namespace detail {

        struct RunBeforeMain {
            RunBeforeMain(void (*func)()) { func(); }
        };

        struct FuncAdd {
            friend void (*operator+(FuncAdd, void (*func)()))() { return func; }
        };
    }  // namespace detail

    std::unique_ptr<Application> create_application();
    void register_application(std::unique_ptr<Application> (*)());
    template <typename T>
        requires std::is_base_of_v<Application, T>
    void register_application() {
        register_application([] -> std::unique_ptr<Application> { return std::make_unique<T>(); });
    }
    void register_mod(std::unique_ptr<AppModule> (*factory)());
    template <typename T>
        requires std::is_base_of_v<AppModule, T>
    void register_mod() {
        register_mod([] -> std::unique_ptr<AppModule> { return std::make_unique<T>(); });
    }
    std::vector<std::unique_ptr<AppModule>> create_mods();
}  // namespace triolib

#define REGISTER_MOD(classname) \
    ;                           \
    run_before_main { ::triolib::register_mod<classname>(); }
#define REGISTER_APP(classname) \
    ;                           \
    run_before_main { ::triolib::register_application<classname>(); }

#define __concat(x, y) x##y
#define _concat(x, y) __concat(x, y)

#define run_before_main static ::triolib::detail::RunBeforeMain _concat(__RUN_BEFORE_MAIN, __COUNTER__) = ::triolib::detail::FuncAdd{} + []

#define _REGISTER(appname)                                               \
    run_before_main {                                                    \
        /* If appname extends off Application, then register application \
         * else register mod*/                                           \
    }

#define tassertmsg(to_check, msg)                                                                                \
    do {                                                                                                         \
        if (!(to_check)) throw ::std::runtime_error("Assert `" #to_check "` has failed: " + ::std::string(msg)); \
    } while (0)

#define tassert(to_check)                                                                  \
    do {                                                                                   \
        if (!(to_check)) throw ::std::runtime_error("Assert `" #to_check "` has failed."); \
    } while (0)
