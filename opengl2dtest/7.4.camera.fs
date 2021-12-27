#version 430 core

in float o_btype;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2DArray texture1;
//uniform sampler2D texture2;

void main()
{
	FragColor = texture(texture1, vec3(TexCoord.xy, o_btype));
	//FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}