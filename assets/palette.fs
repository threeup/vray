// palette.fs - faction-based palettized shading
#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float strength;      // 0 = off, 1 = fully palettized
uniform int paletteIndex;    // 0=Red, 1=Blue, 2=Green, 3=Neutral

// Red Faction
const vec3 RED_HIGHLIGHT = vec3(255.0/255.0, 120.0/255.0, 120.0/255.0);
const vec3 RED_BASE = vec3(232.0/255.0, 32.0/255.0, 32.0/255.0);
const vec3 RED_SHADE = vec3(168.0/255.0, 16.0/255.0, 16.0/255.0);
const vec3 RED_DEEP = vec3(88.0/255.0, 8.0/255.0, 8.0/255.0);

// Blue Faction
const vec3 BLUE_HIGHLIGHT = vec3(144.0/255.0, 184.0/255.0, 255.0/255.0);
const vec3 BLUE_BASE = vec3(40.0/255.0, 96.0/255.0, 232.0/255.0);
const vec3 BLUE_SHADE = vec3(24.0/255.0, 56.0/255.0, 160.0/255.0);
const vec3 BLUE_DEEP = vec3(16.0/255.0, 24.0/255.0, 80.0/255.0);

// Green Faction
const vec3 GREEN_HIGHLIGHT = vec3(160.0/255.0, 240.0/255.0, 136.0/255.0);
const vec3 GREEN_BASE = vec3(56.0/255.0, 184.0/255.0, 48.0/255.0);
const vec3 GREEN_SHADE = vec3(32.0/255.0, 120.0/255.0, 24.0/255.0);
const vec3 GREEN_DEEP = vec3(16.0/255.0, 64.0/255.0, 16.0/255.0);

// Neutral
const vec3 NEUTRAL_HIGHLIGHT = vec3(192.0/255.0, 192.0/255.0, 192.0/255.0);
const vec3 NEUTRAL_BASE = vec3(128.0/255.0, 128.0/255.0, 128.0/255.0);
const vec3 NEUTRAL_SHADE = vec3(96.0/255.0, 96.0/255.0, 104.0/255.0);
const vec3 NEUTRAL_DEEP = vec3(64.0/255.0, 64.0/255.0, 72.0/255.0);

vec3 quantizeAndPalettize(vec3 color, int pIdx) {
    // Get brightness/luminance
    float lum = dot(color, vec3(0.299, 0.587, 0.114));
    
    // Quantize to 4 levels
    float quantized = floor(lum * 3.0) / 3.0;
    
    // Select palette and color based on brightness level
    vec3 palColor;
    if (pIdx == 0) {  // Red
        if (quantized < 0.33) palColor = RED_DEEP;
        else if (quantized < 0.67) palColor = RED_SHADE;
        else if (quantized < 1.0) palColor = RED_BASE;
        else palColor = RED_HIGHLIGHT;
    } else if (pIdx == 1) {  // Blue
        if (quantized < 0.33) palColor = BLUE_DEEP;
        else if (quantized < 0.67) palColor = BLUE_SHADE;
        else if (quantized < 1.0) palColor = BLUE_BASE;
        else palColor = BLUE_HIGHLIGHT;
    } else if (pIdx == 2) {  // Green
        if (quantized < 0.33) palColor = GREEN_DEEP;
        else if (quantized < 0.67) palColor = GREEN_SHADE;
        else if (quantized < 1.0) palColor = GREEN_BASE;
        else palColor = GREEN_HIGHLIGHT;
    } else {  // Neutral
        if (quantized < 0.33) palColor = NEUTRAL_DEEP;
        else if (quantized < 0.67) palColor = NEUTRAL_SHADE;
        else if (quantized < 1.0) palColor = NEUTRAL_BASE;
        else palColor = NEUTRAL_HIGHLIGHT;
    }
    
    return palColor;
}

void main() {
    vec4 base = texture(texture0, fragTexCoord);
    vec3 palettized = quantizeAndPalettize(base.rgb, paletteIndex);
    vec3 mixed = mix(base.rgb, palettized, clamp(strength, 0.0, 1.0));
    finalColor = vec4(mixed, base.a);
}

