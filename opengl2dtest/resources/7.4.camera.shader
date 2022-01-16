#version 330 core

in float o_btype;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2DArray texture1;

void main()
{
	FragColor = texture(texture1, vec3(TexCoord.xy, o_btype));
}