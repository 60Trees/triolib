/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#pragma once

#include <triolib.hpp>

/**
 * @brief Add this as a mod and the `getmultiplier()` function
 * will be evaluated every tick and multiplied with the current
 * deltaTime.
 */
struct SpeedMultiplier : triolib::AppModule {
    void init() override {}
    void loop() override {}
    void quit() override {}
    /**
     * @returns double This function will get set to 1
     * if it is infinity or NaN and will be run through
     * `abs(...)`
     */
    virtual double getmultiplier() = 0;
};

struct FpsHandler : triolib::AppModule {
    /// How many seconds elapsed since the previous frame
    double dt = 0;
    /// How many milliseconds elapsed since the previous frame
    uint64_t ticksElapsed = 0;
    /// How many seconds since the start
    double seconds = 0;
    /// How many milliseconds since the start
    uint64_t ticks = 0;
    bool does_go_before(const triolib::AppModule&) const override { return true; }
};
