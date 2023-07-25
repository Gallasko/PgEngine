#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 scale;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 colors;

out vec3 ourColor;

void main()
{
    gl_Position = projection * view * scale * model * vec4(aPos, 1.0f);
    ourColor = colors;
}