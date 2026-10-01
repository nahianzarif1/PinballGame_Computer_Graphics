#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;

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
uniform int useTexture;
uniform sampler2D textureSampler;

vec3 shadePoint(PointLight light, vec3 norm, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shininess, 1.0));
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.kc + light.kl * distance + light.kq * distance * distance);
    vec3 ambient = light.ambient * light.intensity;
    vec3 diffuse = light.diffuse * diff * light.intensity;
    vec3 specular = light.specular * spec * light.intensity;
    return (ambient + diffuse + specular) * attenuation * albedo;
}

void main() {
    vec3 albedo = objectColor * Color;
    if (useTexture == 1) {
        albedo *= texture(textureSampler, TexCoord).rgb;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = sceneAmbient * albedo;

    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        if (pointLights[i].enabled == 0) continue;
        result += shadePoint(pointLights[i], norm, viewDir, albedo);
    }

    if (spotLight.enabled == 1) {
        vec3 lightDir = normalize(spotLight.position - FragPos);
        float theta = dot(lightDir, normalize(-spotLight.direction));
        float inner = cos(radians(spotLight.cutOff));
        float outer = cos(radians(spotLight.outerCutOff));
        float epsilon = max(inner - outer, 0.0001);
        float intensity = clamp((theta - outer) / epsilon, 0.0, 1.0);
        intensity = pow(intensity, max(spotLight.exponent * 0.05, 0.2));

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

    FragColor = vec4(result, objectAlpha);
}
