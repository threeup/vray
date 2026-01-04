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
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * colDiffuse.rgb;

    // Ambient (minimal specular - 1% metallic, 99% diffuse)
    vec3 ambient = 0.2 * colDiffuse.rgb;
    
    // Extremely subtle specular for barely-visible surface imperfection (0.1% metallic)
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128.0) * 0.01; // 0.01 = 1% metallic
    vec3 specular = spec * vec3(1.0);

    finalColor = vec4(ambient + diffuse + specular, 1.0);
}