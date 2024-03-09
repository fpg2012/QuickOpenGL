#version 430 core

out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D ourTexture;


void main()
{
    vec4 color = texture(ourTexture, texCoord);
    if (color.w < 0.75f) {
        gl_FragDepth = 1.0f;
    }
}