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

vec3 blinnPhong(Material material, PointLight light, vec3 normal, vec3 pos, vec3 eye, vec3 color) {
    float d = length(light.position-pos);
    float intensity = light.intensity / (d*d);
    vec3 view_dire = normalize(eye - pos);
    vec3 light_dire = normalize(light.position - pos);

    vec3 ambient = light.color;
    vec3 diffuse = max(0.0f, dot(light_dire, normal)) * light.color;

    vec3 halfway = normalize(view_dire + light_dire);
    float spec = pow(max(0.0, dot(normal, halfway)), material.phong_exponent);

    vec3 specular = spec * light.color;

    return material.k_ambient * ambient * color + intensity * (material.k_diffuse * diffuse * color + material.k_specular * specular);
}

out vec4 FragColor;
in vec3 pos;
in vec2 texCoord;
in vec3 normal;

uniform sampler2D ourTexture;
uniform PointLight light;
uniform Material material;
uniform vec3 eye;

void main()
{
    vec4 color = texture(ourTexture, texCoord);
    vec3 color3 = color.rgb;
    vec3 phong_color = blinnPhong(material, light, normalize(normal), pos, eye, color3);
    // FragColor = color;
    FragColor = vec4(phong_color, 1.0);
    // FragColor = vec4((normal + 1)/2, 1.0);
}