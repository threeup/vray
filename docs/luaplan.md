# Technical Spec: Lua Configuration System for Raylib

## 1. Overview

We are using **Lua 5.4** as a data-definition language to replace JSON. This allows for logic-based constants (e.g., `gravity * 2`) and eliminates the need to recompile C code when tweaking game balance.

## 2. File Structure: `vars.lua`

The data file must use standard Lua table syntax.

```lua
-- Game Constants
title = "My Tiny Raylib Game"
window = {
    width = 1280,
    height = 720,
    fps = 60
}

player = {
    speed = 5.0,
    jump_force = -12.0,
    color = { r = 230, g = 41, b = 55 } -- Raylib RED
}

```

## 3. C Implementation (The Loader)

To read these values, we use the Lua C API. The core concept is the **Virtual Stack**. To get a value, we push the table onto the stack, then request the field.

### Core Utility Functions

```c
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// Safely loads a float from a global variable
float GetLuaFloat(lua_State* L, const char* varName) {
    lua_getglobal(L, varName);
    if (!lua_isnumber(L, -1)) return 0.0f;
    float val = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);
    return val;
}

// Loads a float from inside a table: table.field
float GetLuaTableFloat(lua_State* L, const char* tableName, const char* fieldName) {
    lua_getglobal(L, tableName);
    if (!lua_istable(L, -1)) return 0.0f;

    lua_getfield(L, -1, fieldName); // Push field onto stack
    float val = (float)lua_tonumber(L, -1);
    lua_pop(L, 2); // Pop field AND table
    return val;
}

```

## 4. Integration Workflow

1. **Initialize:** Create `lua_State* L = luaL_newstate();` and call `luaL_openlibs(L);`.
2. **Execute:** Run `luaL_dofile(L, "vars.lua");` to parse the file into memory.
3. **Fetch:** Use the helpers above to assign values to your C structs during the `Init` phase.
4. **Close:** Always call `lua_close(L);` at the end of the program to prevent memory leaks.

## 5. Hot Reloading Logic (Optional)

To update constants without restarting:

- Check `GetFileModTime("vars.lua")` every 60 frames.
- If the timestamp is newer, call `luaL_dofile` again and re-run your "Fetch" functions.

---

### Pro-Tip for the AI

If you ask the AI to write a specific loader for you, tell it:

> _"Generate a C function using `lua_getfield` to populate a `struct Player` from a Lua table named `player`."_
