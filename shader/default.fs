#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;

void main()
{
	FragColor = vec4(255.0f, 255.0f, 255.0f, 255.0f);
	//FragColor = texture(texture1, TexCoord);
}