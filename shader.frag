#version 430 core

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

struct Material {
    float k_ambient, k_diffuse, k_specular;
    float phong_exponent;
};

vec3 blinnPhong(Material material, PointLight light, vec3 normal, vec3 pos, vec3 eye, vec3 color, float shadow) {
    float d = length(light.position-pos);
    float intensity = light.intensity / (d*d);
    vec3 view_dire = normalize(eye - pos);
    vec3 light_dire = normalize(light.position - pos);

    if (abs(dot(normal, light_dire)) < 0.01) {
        return vec3(1.0f, 1.0f, 0.0f);    
    }

    vec3 ambient = light.color;
    vec3 diffuse = max(0.0f, dot(light_dire, normal)) * light.color;

    vec3 halfway = normalize(view_dire + light_dire);
    float spec = pow(max(0.0, dot(normal, halfway)), material.phong_exponent);

    vec3 specular = spec * light.color;

    return material.k_ambient * ambient * color + (1.0f - shadow) * intensity * (material.k_diffuse * diffuse * color + material.k_specular * specular);
}

out vec4 FragColor;
in vec3 pos;
in vec2 texCoord;
in vec3 normal;
in vec4 pos_light_space;

uniform sampler2D ourTexture;
uniform sampler2D shadowMap;
uniform PointLight light;
uniform Material material;
uniform vec3 eye;

float compareDepth(vec2 coord, float current_depth) {
    float closest_depth = texture(shadowMap, coord).r;
    float shadow = closest_depth + 0.01f < current_depth ? 1.0f : .0f;
    return shadow;
}

float shadowCalc(vec4 p_light_space) {
    vec3 p = p_light_space.xyz / p_light_space.w;
    p = p * 0.5 + 0.5; // NDC [-1, 1] -> [0, 1]
    float current_depth = p.z;
    
    float shadow0 = compareDepth(p.xy, current_depth) * 0.4;
    float shadow1 = compareDepth(p.xy + vec2(1.0f / 1024, 1.0f / 1024), current_depth) * 0.4 * 0.25;
    float shadow2 = compareDepth(p.xy + vec2(1.0f / 1024, -1.0f / 1024), current_depth) * 0.4 * 0.25;
    float shadow3 = compareDepth(p.xy + vec2(-1.0f / 1024, 1.0f / 1024), current_depth) * 0.4 * 0.25;
    float shadow4 = compareDepth(p.xy + vec2(-1.0f / 1024, -1.0f / 1024), current_depth) * 0.4 * 0.25;
    float shadow5 = compareDepth(p.xy + vec2(2.0f / 1024, .0f / 1024), current_depth) * 0.2 * 0.25;
    float shadow6 = compareDepth(p.xy + vec2(.0f / 1024, -2.0f / 1024), current_depth) * 0.2 * 0.25;
    float shadow7 = compareDepth(p.xy + vec2(-2.0f / 1024, .0f / 1024), current_depth) * 0.2 * 0.25;
    float shadow8 = compareDepth(p.xy + vec2(.0f / 1024, 2.0f / 1024), current_depth) * 0.2 * 0.25;

    float shadow = (shadow0 + shadow1 + shadow2 + shadow3 + shadow4 + shadow5 + shadow6 + shadow7 + shadow8);

    return shadow;
}

void main()
{
    vec4 color = texture(ourTexture, texCoord);
    vec3 color3 = color.rgb;
    float shadow = shadowCalc(pos_light_space);
    vec3 phong_color = blinnPhong(material, light, normalize(normal), pos, eye, color3, shadow);
    // FragColor = color;
    FragColor = vec4(phong_color, 1.0);
    // FragColor = vec4((normal + 1)/2, 1.0);
    // FragColor = vec4(vec3(shadow), 1.0f);
}