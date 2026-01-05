#pragma once

#include "raylib.h"

/**
 * @brief RAII wrapper for Shader handles.
 * 
 * Ensures automatic cleanup of shader resources on destruction or move.
 * This is a move-only type to prevent accidental copies of handle references.
 */
struct ShaderHandle {
    Shader shader{0};

    ShaderHandle() = default;

    explicit ShaderHandle(Shader s) : shader(s) {}

    // Destructor: automatically unload shader if valid
    ~ShaderHandle() {
        if (shader.id != 0) {
            UnloadShader(shader);
        }
    }

    // Move constructor: take ownership and zero out the source
    ShaderHandle(ShaderHandle&& other) noexcept : shader(other.shader) {
        other.shader.id = 0;
    }

    // Move assignment: clean up our shader and take ownership of other's
    ShaderHandle& operator=(ShaderHandle&& other) noexcept {
        if (this != &other) {
            if (shader.id != 0) {
                UnloadShader(shader);
            }
            shader = other.shader;
            other.shader.id = 0;
        }
        return *this;
    }

    // Delete copy constructor and copy assignment (move-only)
    ShaderHandle(const ShaderHandle&) = delete;
    ShaderHandle& operator=(const ShaderHandle&) = delete;

    // Check if this handle holds a valid shader
    bool valid() const { return shader.id != 0; }

    // Implicit conversion to Shader for use in raylib calls
    operator Shader() const { return shader; }

    // Explicit access to underlying shader
    Shader get() const { return shader; }
};

/**
 * @brief RAII wrapper for Model handles.
 * 
 * Ensures automatic cleanup of model resources on destruction or move.
 * This is a move-only type to prevent accidental copies of handle references.
 */
struct ModelHandle {
    Model model{0};

    ModelHandle() = default;

    explicit ModelHandle(Model m) : model(m) {}

    // Destructor: automatically unload model if valid
    ~ModelHandle() {
        if (model.meshCount > 0) {
            UnloadModel(model);
        }
    }

    // Move constructor: take ownership and zero out the source
    ModelHandle(ModelHandle&& other) noexcept : model(other.model) {
        other.model.meshCount = 0;
    }

    // Move assignment: clean up our model and take ownership of other's
    ModelHandle& operator=(ModelHandle&& other) noexcept {
        if (this != &other) {
            if (model.meshCount > 0) {
                UnloadModel(model);
            }
            model = other.model;
            other.model.meshCount = 0;
        }
        return *this;
    }

    // Delete copy constructor and copy assignment (move-only)
    ModelHandle(const ModelHandle&) = delete;
    ModelHandle& operator=(const ModelHandle&) = delete;

    // Check if this handle holds a valid model
    bool valid() const { return model.meshCount > 0; }

    // Implicit conversion to Model for use in raylib calls
    operator Model() const { return model; }

    // Explicit access to underlying model
    Model get() const { return model; }
};
