#version 300 es

precision mediump float;
precision highp sampler2DArray;
precision highp float;

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float LightValue;
flat in uint Btype;
in float Lighting;

uniform sampler2DArray atlas;
uniform vec3 cameraPosition;

const float drawDistance = 155.0f;

void AddDrawDistanceFog()
{
	vec4 fogColor = vec4(166.0/255.0, 195.0/255.0, 255.0/255.0, 1.0);
	float fogDistance = drawDistance - 5.0f;
	float fogWallDistance = drawDistance;

    float dist = distance(cameraPosition, FragPos);

    if(dist > fogDistance + 5.0f)
		discard;

    if (dist > fogWallDistance)
    {
        FragColor = fogColor;
    }
    else if (dist > fogDistance) 
    {
        float fogFactor = (dist - fogDistance) / (fogWallDistance - fogDistance);
        FragColor = mix(FragColor, fogColor, fogFactor);
        FragColor.a = 1.0f;
    }
}

void main()
{
    vec4 textureColor = texture(atlas, vec3(TexCoords.xy, Btype));

    if(Btype == 5u && distance(cameraPosition, FragPos) > drawDistance && textureColor.a < 0.7)
    {
        FragColor = vec4(textureColor.xyz * Lighting, 1.0);
    }
    else
    {
        FragColor = vec4(textureColor.xyz * Lighting, textureColor.w);
    }

    AddDrawDistanceFog();
}
