// pastel.fs
#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;

void main() {
    vec4 base = texture(texture0, fragTexCoord);

    // Desaturate slightly
    float gray = dot(base.rgb, vec3(0.3, 0.59, 0.11));
    vec3 desat = mix(base.rgb, vec3(gray), 0.2);

    // Lift brightness for pastel feel
    vec3 pastel = pow(desat, vec3(0.9)) * 1.1;

    finalColor = vec4(pastel, base.a);
}