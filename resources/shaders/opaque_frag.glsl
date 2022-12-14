#version 430 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float LightValue;
in float BType;
in float vert_lighting;
in vec4 DebugColor;

uniform sampler2DArray atlas;

void main()
{    
	if(texture(atlas, vec3(TexCoords.xy, BType)).a < 0.1)
		discard;

    vec4 texture = texture(atlas, vec3(TexCoords.xy, BType));

	if(DebugColor != vec4(0, 0, 0, 0))
		texture *= DebugColor;

	FragColor = vec4(texture.xyz * vert_lighting, texture.w);
}