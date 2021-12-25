#version 430 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2DArray texture1;
//uniform sampler2D texture2;

void main()
{
	FragColor = texture(texture1, vec3(TexCoord.xy, 1));
	//FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}