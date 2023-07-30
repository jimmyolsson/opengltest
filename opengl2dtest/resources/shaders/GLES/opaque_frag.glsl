#version 300 es

precision mediump float;
precision highp sampler2DArray;
precision highp float;

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float LightValue;
in float Btype;
in float Lighting;

uniform sampler2DArray atlas;
uniform vec3 cameraPosition;

void main()
{
    const float drawDistance = 100.0;

    vec4 textureColor = texture(atlas, vec3(TexCoords.xy, Btype));

    if(Btype == 5.0 && distance(cameraPosition, FragPos) > drawDistance && textureColor.a < 0.7)
    {
        FragColor = vec4(textureColor.xyz * Lighting, 1.0);
    }
    else
    {
        FragColor = vec4(textureColor.xyz * Lighting, textureColor.w);
    }
}
