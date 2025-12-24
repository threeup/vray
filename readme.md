# vray-demo

A demo project for a turn-based strategy game where players plan a sequence of actions using cards. Once planned, the sequences for all players are executed simultaneously, and the results play out in a 3D environment.

## Tech Stack

This project is built with C++ and leverages the following technologies:

- **[raylib](https://www.raylib.com/):** For core windowing, input, and 3D rendering.
- **[raygui](https://github.com/raysan5/raygui):** For the immediate-mode graphical user interface.
- **CMake:** For building the project from source.
- **Emscripten:** The project is structured to support exporting to WebAssembly for web-based builds.

## Getting Started

### Prerequisites

- A C++ compiler (like MSVC, GCC, or Clang)
- CMake (must be available in your system's PATH)

### Building the Project

To build the project, run the following commands in PowerShell:

```powershell
# 1. Configure the project and generate build files
cmake -S . -B build

# 2. Compile the project
cmake --build build --config Release
```

Or, use the provided script to build and run all at once:

```powershell
.\go.ps1
```

## run

To run the demo, execute the following command from the project root:

```powershell
.\build\Release\vray_demo.exe
```
