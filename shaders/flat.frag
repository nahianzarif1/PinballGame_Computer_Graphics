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
uniform vec3 sceneAmbient;
uniform float objectAlpha;
uniform int useTexture;
uniform sampler2D textureSampler;

void main() {
    vec3 norm = normalize(Normal);
    vec3 albedo = objectColor * Color;
    if (useTexture == 1) {
        albedo *= texture(textureSampler, TexCoord).rgb;
    }
    vec3 result = sceneAmbient * albedo;

    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        if (pointLights[i].enabled == 0) continue;
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].kc + pointLights[i].kl * distance +
                                   pointLights[i].kq * distance * distance);
        vec3 ambient = pointLights[i].ambient * pointLights[i].intensity;
        vec3 diffuse = pointLights[i].diffuse * diff * pointLights[i].intensity;
        result += (ambient + diffuse) * attenuation * albedo;
    }

    if (spotLight.enabled == 1) {
        vec3 lightDir = normalize(spotLight.position - FragPos);
        float theta = dot(lightDir, normalize(-spotLight.direction));
        float inner = cos(radians(spotLight.cutOff));
        float outer = cos(radians(spotLight.outerCutOff));
        float intensity = clamp((theta - outer) / max(inner - outer, 0.0001), 0.0, 1.0);
        float diff = max(dot(norm, lightDir), 0.0);
        float distance = length(spotLight.position - FragPos);
        float attenuation = 1.0 / (spotLight.kc + spotLight.kl * distance + spotLight.kq * distance * distance);
        result += (spotLight.ambient + spotLight.diffuse * diff * intensity) * spotLight.intensity * attenuation * albedo;
    }

    FragColor = vec4(result, objectAlpha);
}
