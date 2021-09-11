#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

layout (location = 2) in vec4 aMainColor;
layout (location = 4) in vec4 aOutline1;
layout (location = 3) in vec4 aOutline2;
layout (location = 5) in float aEffect;

out vec2 TexCoord;

out vec4 mainColor;
out vec4 outline1;
out vec4 outline2;
out float effect;

out vec4 glPosition;

uniform mat4 model;
uniform mat4 scale;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * scale * model * vec4(aPos, 1.0f);
	//glPosition = projection * view * scale * model * vec4(aPos, 1.0f);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	
	mainColor = aMainColor;
	outline1 = aOutline1;
	outline2 = aOutline2;
	effect = aEffect;
}