// bloom.fs
#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;

void main() {
    vec4 base = texture(texture0, fragTexCoord);

    // Extract bright areas
    float brightness = dot(base.rgb, vec3(0.2126, 0.7152, 0.0722));
    vec3 bloom = brightness > 0.7 ? base.rgb * 1.5 : vec3(0.0);

    // Slight blur (cheap approximation)
    vec3 blurred = (
        texture(texture0, fragTexCoord + vec2(0.002, 0.0)).rgb +
        texture(texture0, fragTexCoord + vec2(-0.002, 0.0)).rgb +
        texture(texture0, fragTexCoord + vec2(0.0, 0.002)).rgb +
        texture(texture0, fragTexCoord + vec2(0.0, -0.002)).rgb
    ) * 0.25;

    finalColor = vec4(base.rgb + bloom + blurred * 0.2, 1.0);
}