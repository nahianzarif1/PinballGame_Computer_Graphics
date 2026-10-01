#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

out vec3 LightingColor;
out vec3 Color;
out float Alpha;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#define MAX_POINT_LIGHTS 4

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
    float kc;
    float kl;
    float kq;
    int enabled;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
    float cutOff;
    float outerCutOff;
    float exponent;
    float kc;
    float kl;
    float kq;
    int enabled;
};

uniform vec3 viewPos;
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 objectColor;
uniform float shininess;
uniform vec3 sceneAmbient;
uniform float objectAlpha;

void main() {
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));
    vec3 norm = normalize(mat3(transpose(inverse(model))) * aNormal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 albedo = objectColor;
    vec3 result = sceneAmbient * albedo;

    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        if (pointLights[i].enabled == 0) continue;
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shininess, 1.0));
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].kc + pointLights[i].kl * distance +
                                   pointLights[i].kq * distance * distance);
        vec3 ambient = pointLights[i].ambient * pointLights[i].intensity;
        vec3 diffuse = pointLights[i].diffuse * diff * pointLights[i].intensity;
        vec3 specular = pointLights[i].specular * spec * pointLights[i].intensity;
        result += (ambient + diffuse + specular) * attenuation * albedo;
    }

    if (spotLight.enabled == 1) {
        vec3 lightDir = normalize(spotLight.position - FragPos);
        float theta = dot(lightDir, normalize(-spotLight.direction));
        float inner = cos(radians(spotLight.cutOff));
        float outer = cos(radians(spotLight.outerCutOff));
        float intensity = clamp((theta - outer) / max(inner - outer, 0.0001), 0.0, 1.0);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shininess, 1.0));
        float distance = length(spotLight.position - FragPos);
        float attenuation = 1.0 / (spotLight.kc + spotLight.kl * distance + spotLight.kq * distance * distance);
        vec3 ambient = spotLight.ambient * spotLight.intensity;
        vec3 diffuse = spotLight.diffuse * diff * spotLight.intensity;
        vec3 specular = spotLight.specular * spec * spotLight.intensity;
        result += (ambient + (diffuse + specular) * intensity) * attenuation * albedo;
    }

    LightingColor = result;
    Color = aColor;
    Alpha = objectAlpha;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
