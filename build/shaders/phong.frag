#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

#define MAX_POINT_LIGHTS 3
#define MAX_SPOT_LIGHTS 2

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
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    int enabled;
};

uniform vec3 viewPos;
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numSpotLights;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform vec3 objectColor;
uniform float shininess;
uniform vec3 sceneAmbient;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = sceneAmbient * objectColor;

    // Process point lights
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

    // Process spot lights
    for (int i = 0; i < numSpotLights && i < MAX_SPOT_LIGHTS; i++) {
        if (spotLights[i].enabled == 0) continue;

        vec3 lightDir = normalize(spotLights[i].position - FragPos);
        float theta = dot(lightDir, normalize(-spotLights[i].direction));
        float epsilon = spotLights[i].cutOff - spotLights[i].outerCutOff;
        float intensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);

        if (theta > spotLights[i].outerCutOff) {
            vec3 ambient = spotLights[i].ambient;
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = spotLights[i].diffuse * diff * intensity;

            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shininess, 1.0));
            vec3 specular = spotLights[i].specular * spec * intensity;

            float distance = length(spotLights[i].position - FragPos);
            float attenuation = 1.0 / (spotLights[i].constant + spotLights[i].linear * distance +
                                       spotLights[i].quadratic * distance * distance);

            result += (ambient + diffuse + specular) * attenuation * objectColor;
        }
    }

    FragColor = vec4(result * Color, 1.0);
}
