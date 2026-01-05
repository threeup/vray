#pragma once

#include <memory>

// Generic state interface parameterized by context (game-like object) and actions payload.
// Each concrete state should implement entry/exit validation and per-frame update.
template <typename Context, typename Actions>
class State {
public:
    virtual ~State() = default;

    // Check if this state can be entered.
    virtual bool canEnter(Context& context) = 0;

    // Check if this state can be exited.
    virtual bool canExit(Context& context) = 0;

    // Called when entering this state.
    virtual void enter(Context& context) = 0;

    // Called when exiting this state.
    virtual void exit(Context& context) = 0;

    // Per-frame update. Return next state to transition; nullptr to stay.
    virtual std::unique_ptr<State> update(Context& context, const Actions& actions, float dt) = 0;

    // Human-readable name for logging/debugging.
    virtual const char* getName() const = 0;
};
