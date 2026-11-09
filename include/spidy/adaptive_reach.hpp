#pragma once

#include <algorithm>
#include <cstdint>

// ADAPTIVE BUILDING DETAIL (--adaptive-building-reach): how far away the far
// buildings draw their full models (stereo_probe.cpp, impostorReach) follows
// whether the game keeps up with the headset. Each headset frame that shows
// the eyes is fresh (a new pair from the game) or reused (the game had none
// ready, so the last pair is shown again). Every `window` such frames, more
// than a sixth reused brings the reach in a step; at most one reused, for
// `calmWindows` windows in a row, takes it back out a step. The scale is a
// percent of the reach the session set (--building-reach), and the reach
// never goes below the game's own distance unless the session set it lower.
namespace spidy::adaptive_reach {

inline constexpr uint32_t window = 45, minimumScale = 50, maximumScale = 100, stepIn = 15, stepOut = 10,
                          calmWindows = 4;

struct State {
    uint32_t scale = maximumScale, frames = 0, reused = 0, calm = 0;
};

// One headset frame that showed the eyes. True when the scale changed: the new
// one is `state.scale`.
constexpr bool frame(State& state, bool reusedPair) {
    ++state.frames;
    state.reused += reusedPair ? 1u : 0u;
    if (state.frames < window)
        return false;
    const uint32_t reused = state.reused, before = state.scale;
    state.frames = state.reused = 0;
    if (reused * 6 > window) {
        state.calm = 0;
        state.scale = std::max(minimumScale, state.scale - std::min(state.scale, stepIn));
    } else if (reused * 33 < window) {
        if (++state.calm >= calmWindows) {
            state.calm = 0;
            state.scale = std::min(maximumScale, state.scale + stepOut);
        }
    } else {
        state.calm = 0;
    }
    return state.scale != before;
}

// The building reach, percent of the game's own distance, for the session's
// `set` reach at `scale` percent of it.
constexpr uint32_t reach(uint32_t set, uint32_t scale) {
    return std::max(std::min(set, 100u), set * std::clamp(scale, minimumScale, maximumScale) / 100);
}

// In the stereo module (stereo_probe.cpp): whether the session asked for it
// (SpidyAdaptiveReach), and the scale the VR loop chose (game_xr.cpp).
bool on();
void apply(uint32_t scale);

} // namespace spidy::adaptive_reach
