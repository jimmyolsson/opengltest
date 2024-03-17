#version 300 es
precision highp float;
precision highp int;

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
    uint direction;
    uint block_type;
    uint lighting_level;
};

VertexInfo extract_bitfield(uint info)
{
    uint u = (info >> 31) & 1u;
    uint v = (info >> 30) & 1u;
    uint ao = (info >> 28) & 3u;
    uint texture_type = (info >> 20) & 255u;
    uint direc = (info >> 19) & 1u;
    uint lighting_level = (info >> 11) & 255u;

    VertexInfo vertex = VertexInfo(u, v, ao, direc, texture_type, lighting_level);

    return vertex;
}

bool is_translucent(uint blockType)
{
    if(blockType == 8u)
        return true;
    else
        return false;
}

float calculate_lighting(uint lighting_flag, uint ambient_occlusion)
{
    const float ao_value = 0.15;

    float ambient_lighting = 0.8;
    if(lighting_flag == 1u)
        ambient_lighting = 0.6;

    return ambient_lighting - (ao_value * float(ambient_occlusion));
}

void main()
{
    VertexInfo vertexInfo = extract_bitfield(in_info);

    vec3 pos = in_pos;
    if(is_translucent(vertexInfo.block_type))
    {
        pos.y -= 0.1;
    }
    vec2 texcoords = vec2(float(vertexInfo.u), float(vertexInfo.v));

    Lighting = (float(vertexInfo.lighting_level) * 0.063) + 0.05;
    Lighting *= calculate_lighting(vertexInfo.direction, vertexInfo.ambient_occlusion);

    FragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);

    TexCoords = texcoords;
    Btype = vertexInfo.block_type;
}
