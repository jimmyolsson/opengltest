#version 430 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float LightValue;
in float Btype;
in float Lighting;

uniform sampler2DArray atlas;

void main()
{    
	if(texture(atlas, vec3(TexCoords.xy, Btype)).a < 0.1)
		discard;

    vec4 texture = texture(atlas, vec3(TexCoords.xy, Btype));

	FragColor = vec4(texture.xyz * Lighting, texture.w);
}