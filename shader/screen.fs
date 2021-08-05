#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
    //FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
} 