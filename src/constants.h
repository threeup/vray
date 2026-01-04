#pragma once

#include "raylib.h"

// Camera constants
const float CAMERA_PITCH = 35.0f; // degrees
const float CAMERA_YAW = 23.0f;   // degrees
const float CAMERA_ROLL = 0.0f;   // degrees

const float CAMERA_FOVY = 45.0f;
const float CAMERA_DISTANCE = 22.0f;

// Camera input sensitivity settings
const float MOVE_SPEED = 15.0f;
const float ROT_SPEED = 2.5f;
const float ZOOM_SPEED = 3.0f;
const float ZOOM_MIN = 5.0f;
const float ZOOM_MAX = 80.0f;