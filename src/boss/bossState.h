#pragma once

#include <memory>
#include "common/state.h"

struct Game;
struct CardActions;

using BossState = State<Game, CardActions>;
