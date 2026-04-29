#version 410 core

layout(location = 0) out vec4 FragColor;

in vec4 Color;
in vec2 Tex;
in vec3 Normal;

uniform sampler2D gTerrainTexture;

uniform vec3 gReversedLightDir;
void main()
{
    vec4 TexColor = texture(gTerrainTexture, Tex);

    vec3 Normal_ = normalize(Normal);
    float Diffuse = dot(Normal_, gReversedLightDir);
    Diffuse = max(0.3f, Diffuse);
    FragColor = Color * TexColor * Diffuse;
}