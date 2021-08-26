#version 330 core

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;

layout (location = 2) out vec4 FragColor;

void main()
{
	FragColor = texture(texture1, TexCoord);
}