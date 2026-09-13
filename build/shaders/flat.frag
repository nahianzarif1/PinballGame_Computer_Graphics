#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

#define MAX_POINT_LIGHTS 3

uniform vec3 viewPos;
uniform int numPointLights;
uniform struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
} pointLights[MAX_POINT_LIGHTS];

uniform vec3 objectColor;

void main() {
    // Flat shading: use the same normal for entire face (no interpolation)
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0);
    
    for (int i = 0; i < numPointLights; i++) {
        if (!pointLights[i].enabled) continue;
        
        // Ambient
        vec3 ambient = pointLights[i].ambient * pointLights[i].intensity;
        
        // Diffuse
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = pointLights[i].diffuse * diff * pointLights[i].intensity;
        
        // Specular (flat - no shininess)
        vec3 specular = vec3(0.0);
        
        // Attenuation
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + 
                                   pointLights[i].quadratic * distance * distance);
        
        ambient *= attenuation;
        diffuse *= attenuation;
        
        result += ambient + diffuse;
    }
    
    vec3 finalColor = (result * objectColor) * Color;
    FragColor = vec4(finalColor, 1.0);
}
