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
in vec3 pos_light_space;

uniform sampler2D ourTexture;
uniform sampler2D shadowMap;
uniform PointLight light;
uniform Material material;
uniform vec3 eye;
uniform float zNear;
uniform float zFar;
uniform float lightSize;
uniform int recvShadow;

float realDepth(float depth) {
    return 1 / (depth * (1/zFar - 1/zNear) + 1/zNear);
}

float compareDepth(vec2 coord, float current_depth, float bias) {
    float closest_depth = texture(shadowMap, coord).r;
    float shadow = closest_depth + bias < current_depth ? 1.0f : .0f;
    return shadow;
}

float blockerDepth(vec2 coord, float current_depth, float bias) {
    float closest_depth = texture(shadowMap, coord).r;
    float blocker_depth = closest_depth + bias < current_depth ? realDepth(closest_depth) : .0f;
    return blocker_depth;
}

float shadowCalc(vec3 p_light_space, float bias) {
    vec3 p = p_light_space;
    p = p * 0.5 + 0.5; // NDC [-1, 1] -> [0, 1]
    float current_depth = p.z;

    float shadow = 0.0f;
    // shadow = compareDepth(p.xy, current_depth, bias);

    int sample_steps = 5;
    float step_size = 10.0f / 1024 / (sample_steps * 2);
    int cnt = 0;

    float weight = 0.0f;
    for (int i = -sample_steps; i <= sample_steps; i++) {
        int max_j = int(sqrt(sample_steps * sample_steps - i*i));
        for (int j = -max_j; j <= max_j; j++) {
                float w = sample_steps - i + sample_steps - j; w = pow(w, 3.0f);
                shadow += compareDepth(vec2(i * step_size, j * step_size) + p.xy, current_depth, bias) * w;
                weight += w;
            // cnt += (i*i + j*j < 16 ? 1 : 0);
        }
    }
    shadow /= weight;

    return shadow;
}

void main()
{
    vec4 color = texture(ourTexture, texCoord);
    vec3 light_dire = normalize(light.position - pos);

    // if (abs(dot(normalize(normal), light_dire)) < 0.01) {
    //     FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    //     return;  
    // }
    // if (color.w < 0.9f) {
    //     gl_FragDepth = 1.0f;
    // }
    float shadow = 0.0f;
    float bias = 0.001f;  
    if (recvShadow != 0) {
        shadow = shadowCalc(pos_light_space, bias);
    }
    vec3 phong_color = blinnPhong(material, light, normalize(normal), pos, eye, color.rgb, shadow);
    FragColor = vec4((1.5 - shadow) * color.rgb, color.w);
    // FragColor = vec4(phong_color, 1.0);
    // FragColor = vec4((normal + 1)/2, 1.0);
    // FragColor = vec4(vec3(shadow), 1.0f);
}