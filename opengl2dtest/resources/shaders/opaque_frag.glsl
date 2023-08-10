#version 430 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float LightValue;
flat in uint Btype;
in float Lighting;

uniform sampler2DArray atlas;
uniform vec3 cameraPosition;

const float drawDistance = 155.0f;

void AddFog()
{
    vec4 fogColor = vec4(166.0/255.0, 195.0/255.0, 255.0/255.0, 1.0);
    float fogDistance = drawDistance - 5;
    float fogWallDistance = drawDistance;

    float dist = length(cameraPosition - FragPos);

    if (dist > fogWallDistance)
    {
        FragColor = fogColor;
    }
    else if (dist > fogDistance) 
    {
        float fogFactor = (dist - fogDistance) / (fogWallDistance - fogDistance);
        FragColor = mix(FragColor, fogColor, fogFactor);
    }
}

void main()
{ 
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

	AddFog();
}
