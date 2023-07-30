#version 430 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float LightValue;
flat in uint Btype;
in float Lighting;

uniform sampler2DArray atlas;
uniform vec3 cameraPosition;

void main()
{ 
	const float drawDistance = 100.0f;

	vec4 texture = texture(atlas, vec3(TexCoords.xy, Btype));

	// Btype 5 = leaves
	// Make them opaque from a certain distance
	if(Btype == 5 && distance(cameraPosition, FragPos) > drawDistance && texture.a < 0.7)
	{
		FragColor = vec4(texture.xyz * Lighting, 1.0);
	}
	else
	{
		FragColor = vec4(texture.xyz * Lighting, texture.w);
	}
}
