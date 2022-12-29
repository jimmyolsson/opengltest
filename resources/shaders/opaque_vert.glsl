#version 430 core
layout (location = 0) in uint aInfo;

out vec3 FragPos;
out vec2 TexCoords;
out float BType;
out float vert_lighting;
out float border;
out vec4 DebugColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int enabled;

uniform vec4 debug_color;

void main()
{
	uint ax = (aInfo & 0xFF000000) >> 24;
	uint ay = (aInfo & 0x00FF0000) >> 16;
	uint az = (aInfo & 0x0000FF00) >> 8;
	uint au = (aInfo & 0x00000080) >> 7;
	uint av = (aInfo & 0x00000040) >> 6;
	uint ab = (aInfo & 0x0000003F) >> 2;
	uint am = (aInfo & 0x00000003);

    vec3 aPos = vec3(ax, ay, az);
    vec2 aTexCoords = vec2(au, av);
    float aBType = ab;

    if(ax >= 30 || ax == 0 || az >= 31 || az == 0)
		border = 1;
    else
        border = 0;

    if(enabled == 0)
        am = 0;

    vert_lighting = 0.8f - 0.2f * am;

    DebugColor = debug_color;

    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoords = aTexCoords;
    BType = aBType;
}