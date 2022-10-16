#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 aTex;

uniform mat4 transform;

out vec2 TexCoords;

void main() 
{
    gl_Position = transform * vec4(position, 1);
    TexCoords = aTex;
}