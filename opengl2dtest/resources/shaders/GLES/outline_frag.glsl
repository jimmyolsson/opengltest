#version 300 es

precision mediump float;

out vec4 FragColor;

in vec2 TexCoord;

void main()
{
    if (TexCoord.x > 0.01 && TexCoord.y > 0.01 && TexCoord.x < 0.99 && TexCoord.y < 0.99)
        discard;

    gl_FragDepth = gl_FragCoord.z - 0.00001;
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}