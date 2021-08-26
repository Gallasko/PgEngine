#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in float visible;

// texture samplers
uniform sampler2D texture1;

void main()
{
	if(visible > 0.0f)
		FragColor = vec4(255.0f, 255.0f, 255.0f, 255.0f);
		//FragColor = texture(texture1, TexCoord);
	else
		discard;
		//FragColor = vec4(255.0f, 0.0f, 0.0f, 255.0f);
}