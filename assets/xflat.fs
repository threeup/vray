#version 330

in vec3 fragPosition;
flat in vec3 fragNormal; // <--- Must match the vertex shader

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec4 colDiffuse;

// Palette controls
uniform int paletteEnabled;      // 0 = off, 1 = apply palette
uniform int paletteIndex;        // 0=Red, 1=Blue, 2=Green, 3=Neutral
uniform float paletteStrength;   // 0..1 blend amount

out vec4 finalColor;

// Palette constants (match palette.fs)
const vec3 RED_HIGHLIGHT   = vec3(255.0/255.0, 120.0/255.0, 120.0/255.0);
const vec3 RED_BASE        = vec3(232.0/255.0,  32.0/255.0,  32.0/255.0);
const vec3 RED_SHADE       = vec3(168.0/255.0,  16.0/255.0,  16.0/255.0);
const vec3 RED_DEEP        = vec3( 88.0/255.0,   8.0/255.0,   8.0/255.0);

const vec3 BLUE_HIGHLIGHT  = vec3(144.0/255.0, 184.0/255.0, 255.0/255.0);
const vec3 BLUE_BASE       = vec3( 40.0/255.0,  96.0/255.0, 232.0/255.0);
const vec3 BLUE_SHADE      = vec3( 24.0/255.0,  56.0/255.0, 160.0/255.0);
const vec3 BLUE_DEEP       = vec3( 16.0/255.0,  24.0/255.0,  80.0/255.0);

const vec3 GREEN_HIGHLIGHT = vec3(160.0/255.0, 240.0/255.0, 136.0/255.0);
const vec3 GREEN_BASE      = vec3( 56.0/255.0, 184.0/255.0,  48.0/255.0);
const vec3 GREEN_SHADE     = vec3( 32.0/255.0, 120.0/255.0,  24.0/255.0);
const vec3 GREEN_DEEP      = vec3( 16.0/255.0,  64.0/255.0,  16.0/255.0);

const vec3 NEUTRAL_HIGHLIGHT = vec3(192.0/255.0, 192.0/255.0, 192.0/255.0);
const vec3 NEUTRAL_BASE      = vec3(128.0/255.0, 128.0/255.0, 128.0/255.0);
const vec3 NEUTRAL_SHADE     = vec3( 96.0/255.0,  96.0/255.0, 104.0/255.0);
const vec3 NEUTRAL_DEEP      = vec3( 64.0/255.0,  64.0/255.0,  72.0/255.0);

vec3 selectPaletteColor(int pIdx, float quantized) {
    if (pIdx == 0) { // Red
        if (quantized < 0.33) return RED_DEEP;
        else if (quantized < 0.67) return RED_SHADE;
        else if (quantized < 1.0) return RED_BASE;
        else return RED_HIGHLIGHT;
    } else if (pIdx == 1) { // Blue
        if (quantized < 0.33) return BLUE_DEEP;
        else if (quantized < 0.67) return BLUE_SHADE;
        else if (quantized < 1.0) return BLUE_BASE;
        else return BLUE_HIGHLIGHT;
    } else if (pIdx == 2) { // Green
        if (quantized < 0.33) return GREEN_DEEP;
        else if (quantized < 0.67) return GREEN_SHADE;
        else if (quantized < 1.0) return GREEN_BASE;
        else return GREEN_HIGHLIGHT;
    } else { // Neutral
        if (quantized < 0.33) return NEUTRAL_DEEP;
        else if (quantized < 0.67) return NEUTRAL_SHADE;
        else if (quantized < 1.0) return NEUTRAL_BASE;
        else return NEUTRAL_HIGHLIGHT;
    }
}

vec3 applyPalette(vec3 litColor) {
    float lum = dot(litColor, vec3(0.299, 0.587, 0.114));
    float quantized = floor(lum * 3.0) / 3.0; // 4 steps: 0, 0.33, 0.66, 1.0
    vec3 palColor = selectPaletteColor(paletteIndex, quantized);
    float blend = clamp(paletteStrength, 0.0, 1.0);
    return mix(litColor, palColor, blend);
}

void main() {
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);
    
    // Wrap lighting to soften banding on flat faces
    float ndl = dot(norm, lightDir);
    float diff = clamp((ndl + 0.35) / 1.35, 0.0, 1.0); // shifts a bit of light into the terminator
    vec3 diffuse = diff * colDiffuse.rgb;

    // Lift ambient so occluded areas still read
    vec3 ambient = 0.35 * colDiffuse.rgb;
    
    // Subtle specular: lower intensity and exponent to avoid tiny hot spots on weapons
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0) * 0.003; // 0.3% metallic
    vec3 specular = spec * vec3(1.0);

    vec3 litColor = ambient + diffuse + specular;
    if (paletteEnabled != 0) {
        litColor = applyPalette(litColor);
    }

    finalColor = vec4(litColor, 1.0);
}