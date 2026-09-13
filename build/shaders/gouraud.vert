#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 LightingColor;
out vec3 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

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
uniform float shininess;

void main() {
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));
    vec3 norm = mat3(transpose(inverse(model))) * aNormal;
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
        
        // Specular (Gouraud - calculated per vertex)
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = pointLights[i].specular * spec * pointLights[i].intensity;
        
        // Attenuation
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + 
                                   pointLights[i].quadratic * distance * distance);
        
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        
        result += ambient + diffuse + specular;
    }
    
    LightingColor = result * objectColor;
    Color = aColor;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
