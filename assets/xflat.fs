#version 330

in vec3 fragPosition;
flat in vec3 fragNormal; // <--- Must match the vertex shader

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec4 colDiffuse;

out vec4 finalColor;

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

    finalColor = vec4(ambient + diffuse + specular, 1.0);
}