#include "camControl.h"
#include "constants.h"
#include <cmath>
#include <raylib.h>

// ----------------------------------------------------------------------------------
// State Management
// ----------------------------------------------------------------------------------
// We initialize these DIRECTLY from your constants.h file.
// We use static variables so the camera "remembers" its angle/zoom between frames.
static float currentDistance = CAMERA_DISTANCE;
// Convert degrees to radians for internal math
static float currentPitch    = CAMERA_PITCH * (PI / 180.0f); 
static float currentYaw      = CAMERA_YAW * (PI / 180.0f);

void initializeCamera(Camera3D& camera) {
    // 1. Set static properties from constants
    camera.target = { 0.0f, 0.0f, 0.0f }; // Look at world center initially
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = CAMERA_FOVY;
    camera.projection = CAMERA_PERSPECTIVE;

    // 2. Reset mutable state to constants (in case we re-init)
    currentDistance = CAMERA_DISTANCE;
    currentPitch    = CAMERA_PITCH * (PI / 180.0f);
    currentYaw      = CAMERA_YAW * (PI / 180.0f);

    // 3. Force position update immediately so it starts in the right spot
    updateCamera(camera);
}

void updateCamera(Camera3D& camera) {
    float dt = GetFrameTime();

    // ----------------------------------------------------------------------
    // 1. ZOOM (Mouse Wheel)
    // ----------------------------------------------------------------------
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        currentDistance -= wheel * ZOOM_SPEED;
        
        // Clamp zoom
        if (currentDistance < ZOOM_MIN) currentDistance = ZOOM_MIN;
        if (currentDistance > ZOOM_MAX) currentDistance = ZOOM_MAX;
    }

    // ----------------------------------------------------------------------
    // 2. ORBIT ROTATION (Q / E)
    // ----------------------------------------------------------------------
    if (IsKeyDown(KEY_Q)) currentYaw -= ROT_SPEED * dt;
    if (IsKeyDown(KEY_E)) currentYaw += ROT_SPEED * dt;

    // Optional: Pitch adjustment (R / F) - clamped to prevent flipping
    if (IsKeyDown(KEY_R)) currentPitch -= ROT_SPEED * dt;
    if (IsKeyDown(KEY_F)) currentPitch += ROT_SPEED * dt;
    
    // Clamp pitch: Keep it between 5 degrees and 89 degrees (never strictly 90)
    float minPitch = 5.0f * (PI / 180.0f);
    float maxPitch = 70.0f * (PI / 180.0f);
    if (currentPitch < minPitch) currentPitch = minPitch;
    if (currentPitch > maxPitch) currentPitch = maxPitch;

    // ----------------------------------------------------------------------
    // 3. PANNING (WASD) - Relative to Camera View
    // ----------------------------------------------------------------------
    // To move "Forward" relative to the camera, we need the flat direction 
    // on the XZ plane based on the current Yaw.
    
    // Forward vector (flat)
    float fwdX = sinf(currentYaw);
    float fwdZ = cosf(currentYaw);

    // Right vector (flat) - 90 degrees offset from forward
    float rightX = sinf(currentYaw + PI / 2.0f);
    float rightZ = cosf(currentYaw + PI / 2.0f);

    if (IsKeyDown(KEY_W)) {
        camera.target.x -= fwdX * MOVE_SPEED * dt;
        camera.target.z -= fwdZ * MOVE_SPEED * dt;
    }
    if (IsKeyDown(KEY_S)) {
        camera.target.x += fwdX * MOVE_SPEED * dt;
        camera.target.z += fwdZ * MOVE_SPEED * dt;
    }
    if (IsKeyDown(KEY_A)) {
        camera.target.x -= rightX * MOVE_SPEED * dt;
        camera.target.z -= rightZ * MOVE_SPEED * dt;
    }
    if (IsKeyDown(KEY_D)) {
        camera.target.x += rightX * MOVE_SPEED * dt;
        camera.target.z += rightZ * MOVE_SPEED * dt;
    }

    // ----------------------------------------------------------------------
    // 4. RECALCULATE POSITION (Spherical Coordinates)
    // ----------------------------------------------------------------------
    // Standard spherical to cartesian conversion
    // x = r * sin(theta) * cos(phi)
    // y = r * cos(theta)
    // z = r * sin(theta) * sin(phi)
    // (Adjusted for Y-up coordinate system where Pitch is angle from ground)
    
    float hDistance = currentDistance * cosf(currentPitch); // Horizontal dist
    float vDistance = currentDistance * sinf(currentPitch); // Vertical dist (height)

    float offsetX = hDistance * sinf(currentYaw);
    float offsetZ = hDistance * cosf(currentYaw);

    camera.position.x = camera.target.x + offsetX;
    camera.position.y = camera.target.y + vDistance;
    camera.position.z = camera.target.z + offsetZ;
}