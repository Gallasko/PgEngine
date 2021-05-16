#version 330 core

in vec2 TexCoord;

in vec4 mainColor;
in vec4 outline1;
in vec4 outline2;
in float effect;

// texture samplers
uniform sampler2D texture1;
uniform int time;

void main()
{
	vec4 pixelColor = texture(texture1, TexCoord); 

	if(pixelColor.a == (255.0 / 255.0)) // Charactere Color
	{
		//pixelColor = vec4(abs(sin(time / 1000.0)), 0.0, 0.0, 1.0);
		pixelColor = mainColor;
	}
	else if (pixelColor.a == (180.0 / 255.0)) // First Outline
	{
		pixelColor = outline1;
	}
	else if (pixelColor.a == (205.0 / 255.0)) // Second Outline
	{
		pixelColor = outline2;
	}

	gl_FragColor = pixelColor;

}