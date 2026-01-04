// palette.fs - palettized shading with soft blend
#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float strength; // 0 = off, 1 = fully palettized

vec3 pickNearest(vec3 color) {
    // Curated 5-color palette (deep dusk, sand, moss, teal, highlight)
    const vec3 palette[5] = vec3[](
        vec3(0.08, 0.10, 0.14), // charcoal blue
        vec3(0.82, 0.73, 0.56), // warm sand
        vec3(0.39, 0.54, 0.33), // moss green
        vec3(0.32, 0.69, 0.72), // soft teal
        vec3(0.95, 0.84, 0.60)  // warm highlight
    );

    float bestDist = 1e9;
    vec3 best = palette[0];
    for (int i = 0; i < 5; ++i) {
        float d = distance(color, palette[i]);
        if (d < bestDist) {
            bestDist = d;
            best = palette[i];
        }
    }
    return best;
}

void main() {
    vec4 base = texture(texture0, fragTexCoord);
    vec3 nearest = pickNearest(base.rgb);
    vec3 mixed = mix(base.rgb, nearest, clamp(strength, 0.0, 1.0));
    finalColor = vec4(mixed, base.a);
}
