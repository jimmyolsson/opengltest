#version 430 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main() 
{
//    if (TexCoords.x > 0.01f && TexCoords.y > 0.01f && TexCoords.x < 0.99f && TexCoords.y < 0.99f)
//		discard;

    FragColor = texture(texture1, TexCoords);
//    FragColor = vec4(0, 1, 0, 1.0);
}