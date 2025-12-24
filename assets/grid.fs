#version 330

// Screen-aligned UV stores world-space XZ baked into the mesh texcoords.
in vec2 fragTexCoord;
out vec4 finalColor;

uniform vec2 gridSize;   // Number of cells (e.g., 12 x 12)
uniform float lineWidth; // World-units thickness (0.0 - 0.5)
uniform vec4 lineColor;
uniform vec4 bgColor;

float grid_mask(vec2 worldXZ) {
    // Shift so cell 0 starts at -(gridSize/2) - 0.5 to match entity placement.
    vec2 cell = worldXZ + (gridSize * 0.5) + 0.5;
    vec2 distToLine = min(fract(cell), 1.0 - fract(cell));
    vec2 aa = fwidth(cell);
    float w = clamp(lineWidth, 0.01, 0.45);
    vec2 lower = vec2(w);
    vec2 upper = vec2(w) + aa;
    vec2 axis = 1.0 - smoothstep(lower, upper, distToLine);
    return max(axis.x, axis.y);
}

void main() {
    float mask = grid_mask(fragTexCoord);
    finalColor = mix(bgColor, lineColor, mask);
}