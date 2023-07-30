#version 430 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in uint in_info;

out vec3 FragPos;
out vec2 TexCoords;
flat out uint Btype;
out float Lighting;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct VertexInfo
{
    float y;
    uint x;
    uint z;
    uint u;
    uint v;
    uint ambient_occlusion;
    uint block_type;
    uint lighting_flag;
};

VertexInfo extract_bitfield(uint vinfo)
{
	float ay = (vinfo & 0xFF000000) >> 24;
	uint ax = (vinfo & 0x00FC0000) >> 18;
	uint az = (vinfo & 0x0003F000) >> 12;
	uint au = (vinfo & 0x00000800) >> 11;
	uint av = (vinfo & 0x00000400) >> 10;
	uint am = (vinfo & 0x00000300) >> 8;
	uint ab = (vinfo & 0x000000FE) >> 1;
	uint al = (vinfo & 0x00000001);

	VertexInfo vertex = VertexInfo(ay, ax, az, au, av, am, ab, al);
    return vertex;
}

bool is_translucent(uint blockType)
{
    // Water
    if(blockType == 9)
		return true;
    else
		return false;
}

float calculate_lighting(uint lighting_flag, uint ambient_occlusion)
{
    const float ao_value = 0.15f;

	float ambient_lighting = 0.8f;
    if(lighting_flag == 1)
		ambient_lighting = 0.6f;

    return ambient_lighting - (ao_value * ambient_occlusion);
}

void main()
{
    VertexInfo vertexInfo = extract_bitfield(in_info);

    if(is_translucent(vertexInfo.block_type))
    {
		vertexInfo.y -= 0.1;
    }
    vec3 pos = in_pos;
    vec2 texcoords = vec2(vertexInfo.u, vertexInfo.v);

    Lighting = calculate_lighting(vertexInfo.lighting_flag, vertexInfo.ambient_occlusion);

    FragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);

    TexCoords = texcoords;
    Btype = vertexInfo.block_type;
}