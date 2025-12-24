#include "camControl.h"
#include "constants.h"
#include <cmath>

static Vector3 baseOffset;

void initializeCamera(Camera3D& camera) {
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.up = {0.0f, 1.0f, 0.0f};

    // Compute initial position from constants
    float pitchRad = CAMERA_PITCH * acosf(-1.0f) / 180.0f;
    float yawRad = CAMERA_YAW * acosf(-1.0f) / 180.0f;

    camera.position.x = camera.target.x + CAMERA_DISTANCE * cosf(pitchRad) * cosf(yawRad);
    camera.position.y = camera.target.y + CAMERA_DISTANCE * sinf(pitchRad);
    camera.position.z = camera.target.z + CAMERA_DISTANCE * cosf(pitchRad) * sinf(yawRad);

    // Deduce base offset
    baseOffset.x = camera.position.x - camera.target.x;
    baseOffset.y = camera.position.y - camera.target.y;
    baseOffset.z = camera.position.z - camera.target.z;
}

void updateCamera(Camera3D& camera) {
    float time = (float)GetTime();
    float yawAngle = 5.0f * sinf(2.0f * acosf(-1.0f) * time / 30.0f) * acosf(-1.0f) / 180.0f; 

    // Use deduced base offset
    Vector3 rotatedOffset;
    rotatedOffset.x = baseOffset.x * cosf(yawAngle) - baseOffset.z * sinf(yawAngle);
    rotatedOffset.y = baseOffset.y;
    rotatedOffset.z = baseOffset.x * sinf(yawAngle) + baseOffset.z * cosf(yawAngle);

    camera.position.x = camera.target.x + rotatedOffset.x;
    camera.position.y = camera.target.y + rotatedOffset.y;
    camera.position.z = camera.target.z + rotatedOffset.z;
}