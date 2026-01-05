#pragma once

#include "state.h"
#include <memory>
#include <string>
#include <raylib.h>

// Generic state machine orchestrator that owns the current state and
// enforces validation on transitions.
template <typename Context, typename Actions>
class StateMachine {
public:
    using StateType = State<Context, Actions>;

    explicit StateMachine(std::string name = "StateMachine") : name_(std::move(name)) {}

    // Initialize with a starting state.
    void begin(Context& context, std::unique_ptr<StateType> initial) {
        if (!initial) {
            TraceLog(LOG_ERROR, "[%s] begin() called with null initial state", name_.c_str());
            return;
        }
        if (!initial->canEnter(context)) {
            TraceLog(LOG_ERROR, "[%s] Initial state %s rejected canEnter()", name_.c_str(), initial->getName());
            return;
        }
        initial->enter(context);
        currentState_ = std::move(initial);
        TraceLog(LOG_INFO, "[%s] Initialized with state %s", name_.c_str(), currentState_->getName());
    }

    // Per-frame update. Delegates to current state and applies transitions.
    void update(Context& context, const Actions& actions, float dt) {
        if (!currentState_) {
            TraceLog(LOG_WARNING, "[%s] update() called with no active state", name_.c_str());
            return;
        }

        auto nextState = currentState_->update(context, actions, dt);
        if (nextState) {
            transitionToState(std::move(nextState), context);
        }
    }

    StateType* getCurrentState() const { return currentState_.get(); }

    const char* getCurrentStateName() const {
        return currentState_ ? currentState_->getName() : "UNKNOWN";
    }

private:
    std::string name_;
    std::unique_ptr<StateType> currentState_;

    void transitionToState(std::unique_ptr<StateType> nextState, Context& context) {
        if (!nextState) {
            TraceLog(LOG_DEBUG, "[%s] transition requested with null next state; staying in %s", name_.c_str(), getCurrentStateName());
            return;
        }

        TraceLog(LOG_DEBUG, "[%s] Attempting transition: %s -> %s", name_.c_str(), getCurrentStateName(), nextState->getName());

        if (!currentState_->canExit(context)) {
            TraceLog(LOG_WARNING, "[%s] Transition blocked: cannot exit %s", name_.c_str(), currentState_->getName());
            return;
        }

        if (!nextState->canEnter(context)) {
            TraceLog(LOG_WARNING, "[%s] Transition blocked: cannot enter %s", name_.c_str(), nextState->getName());
            return;
        }

        currentState_->exit(context);
        nextState->enter(context);
        currentState_ = std::move(nextState);

        TraceLog(LOG_INFO, "[%s] Transition success: now in %s", name_.c_str(), currentState_->getName());
    }
};
