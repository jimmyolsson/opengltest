#version 430 core

out vec4 FragColor;

in vec2 TexCoord;

void main() 
{
    if (TexCoord.x > 0.01f && TexCoord.y > 0.01f && TexCoord.x < 0.99f && TexCoord.y < 0.99f)
		discard;

    gl_FragDepth = gl_FragCoord.z - 0.00001f;
    FragColor = vec4(0, 0, 0, 1);
}