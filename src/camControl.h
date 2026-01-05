#pragma once

#include "raylib.h"

// Forward declaration
struct AppConfig;

void initializeCamera(Camera3D& camera);
void initializeCameraWithConfig(Camera3D& camera, const AppConfig& config);
void updateCamera(Camera3D& camera);
void updateCameraWithConfig(Camera3D& camera, const AppConfig& config);