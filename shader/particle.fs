#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in float visible;

// texture samplers
uniform sampler2D texture1;

void main()
{
    if(!visible)
        discard;
    
	FragColor = texture(texture1, TexCoord);
}