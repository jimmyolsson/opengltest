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
    uint u;
    uint v;
    uint ambient_occlusion;
    // This is BlockTextureIndex, NOT BlockType
    uint direction;
    uint block_type;
    uint lighting_level;
};

VertexInfo extract_bitfield(uint info)
{
	uint u = bitfieldExtract(info, 31, 1);
	uint v = bitfieldExtract(info, 30, 1);
	uint ao = bitfieldExtract(info, 28, 2);
	uint texture_type = bitfieldExtract(info, 20, 8);
	uint direc = bitfieldExtract(info, 19, 1);
	uint lighting_level = bitfieldExtract(info, 11, 8);

	VertexInfo vertex = VertexInfo(u, v, ao, direc, texture_type, lighting_level);

    return vertex;
}

bool is_translucent(uint blockType)
{
    // Water
    if(blockType == 8)
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

    vec3 pos = in_pos;
    if(is_translucent(vertexInfo.block_type))
    {
		pos.y -= 0.1;
    }
    vec2 texcoords = vec2(vertexInfo.u, vertexInfo.v);

    Lighting = (vertexInfo.lighting_level * 0.063) + 0.05;
    Lighting *= calculate_lighting(vertexInfo.direction, vertexInfo.ambient_occlusion);

    FragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);

    TexCoords = texcoords;
    Btype = vertexInfo.block_type;
}