// fxaa.fs - simple FXAA pass
#version 330

in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform vec2 resolution; // screen resolution in pixels
out vec4 finalColor;

// Adapted lightweight FXAA; good enough for post-AA on thin edges
void main() {
    vec2 rcp = 1.0 / resolution;

    vec3 rgbNW = texture(texture0, fragTexCoord + rcp * vec2(-1.0, -1.0)).rgb;
    vec3 rgbNE = texture(texture0, fragTexCoord + rcp * vec2( 1.0, -1.0)).rgb;
    vec3 rgbSW = texture(texture0, fragTexCoord + rcp * vec2(-1.0,  1.0)).rgb;
    vec3 rgbSE = texture(texture0, fragTexCoord + rcp * vec2( 1.0,  1.0)).rgb;
    vec3 rgbM  = texture(texture0, fragTexCoord).rgb;

    float lumaNW = dot(rgbNW, vec3(0.299, 0.587, 0.114));
    float lumaNE = dot(rgbNE, vec3(0.299, 0.587, 0.114));
    float lumaSW = dot(rgbSW, vec3(0.299, 0.587, 0.114));
    float lumaSE = dot(rgbSE, vec3(0.299, 0.587, 0.114));
    float lumaM  = dot(rgbM , vec3(0.299, 0.587, 0.114));

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.03125, 0.0078125);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, vec2(-8.0), vec2(8.0)) * rcp;

    vec3 rgbA = 0.5 * (
        texture(texture0, fragTexCoord + dir * (1.0/3.0 - 0.5)).rgb +
        texture(texture0, fragTexCoord + dir * (2.0/3.0 - 0.5)).rgb
    );
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(texture0, fragTexCoord + dir * -0.5).rgb +
        texture(texture0, fragTexCoord + dir * 0.5).rgb
    );

    float lumaB = dot(rgbB, vec3(0.299, 0.587, 0.114));
    finalColor = (lumaB < lumaMin || lumaB > lumaMax) ? vec4(rgbA, 1.0) : vec4(rgbB, 1.0);
}

