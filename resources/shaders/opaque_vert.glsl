#version 430 core
layout (location = 0) in uint aInfo;

out vec3 FragPos;
out vec2 TexCoords;
out float BType;
out float vert_lighting;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int enabled;

uniform vec4 debug_color;

bool is_translucent(uint blockType)
{
    // Water
    if(blockType == 9)
		return true;
    else
		return false;
}

void main()
{
	float ay = (aInfo & 0xFF000000) >> 24;
	uint ax = (aInfo & 0x00FC0000) >> 18;
	uint az = (aInfo & 0x0003F000) >> 12;
	uint au = (aInfo & 0x00000800) >> 11;
	uint av = (aInfo & 0x00000400) >> 10;
	uint am = (aInfo & 0x00000300) >> 8;
	uint ab = (aInfo & 0x000000FE) >> 1;
	uint al = (aInfo & 0x00000001);

    if(is_translucent(ab))
    {
		ay -= 0.1;
    }

    vec3 aPos = vec3(ax, ay, az);
    vec2 aTexCoords = vec2(au, av);
    float aBType = ab;


    if(enabled == 0)
        am = 0;

	float lighting = 0.8f;
    if(al == 1)
		lighting = 0.6f;

    vert_lighting = lighting - (0.15f * am);

    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoords = aTexCoords;
    BType = aBType;
}