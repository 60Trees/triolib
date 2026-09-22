/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#include "triolib.hpp"

namespace triolib {

    namespace {

        using ModFactory = std::unique_ptr<AppModule> (*)();
        using ApplicationFactory = std::unique_ptr<Application> (*)();

        // All registered module factories.
        std::vector<ModFactory>& mod_factories() {
            static std::vector<ModFactory> factories;
            return factories;
        }

        // There should normally only be one Application.
        ApplicationFactory& application_factory() {
            static ApplicationFactory factory = nullptr;
            return factory;
        }

    }  // namespace

    // -------------------------------------------------------------------------
    // Modules
    // -------------------------------------------------------------------------

    void register_mod(ModFactory factory) {
        if (factory == nullptr) throw std::runtime_error("Cannot register a null application module factory.");

        mod_factories().push_back(factory);
    }

    std::vector<std::unique_ptr<AppModule>> create_mods() {
        std::vector<std::unique_ptr<AppModule>> mods;

        const auto& factories = mod_factories();

        mods.reserve(factories.size());

        for (ModFactory factory : factories) {
            if (factory == nullptr) throw std::runtime_error("Application module registry contains a null factory.");

            mods.emplace_back(factory());
        }

        return mods;
    }

    // Application::mods uses this function during construction.
    std::vector<std::unique_ptr<AppModule>> get_app_mods() { return create_mods(); }

    // -------------------------------------------------------------------------
    // Application
    // -------------------------------------------------------------------------

    void register_application(ApplicationFactory factory) {
        if (factory == nullptr) throw std::runtime_error("Cannot register a null application factory.");

        if (application_factory() != nullptr) throw std::runtime_error("An application has already been registered.");

        application_factory() = factory;
    }

    std::unique_ptr<Application> create_application() {
        ApplicationFactory factory = application_factory();

        if (factory == nullptr) {
            throw std::runtime_error("No application has been registered.");
        }

        auto application = factory();

        if (!application) throw std::runtime_error("Registered application factory returned nullptr.");

        return application;
    }
}  // namespace triolib
