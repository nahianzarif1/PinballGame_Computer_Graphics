#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

#define MAX_POINT_LIGHTS 3

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

uniform vec3 viewPos;
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform vec3 objectColor;
uniform float shininess;
uniform vec3 sceneAmbient;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = sceneAmbient * objectColor;

    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        if (pointLights[i].enabled == 0) continue;

        vec3 ambient = pointLights[i].ambient * pointLights[i].intensity;
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = pointLights[i].diffuse * diff * pointLights[i].intensity;

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shininess, 1.0));
        vec3 specular = pointLights[i].specular * spec * pointLights[i].intensity;

        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].kc + pointLights[i].kl * distance +
                                   pointLights[i].kq * distance * distance);

        result += (ambient + diffuse + specular) * attenuation * objectColor;
    }

    FragColor = vec4(result * Color, 1.0);
}
