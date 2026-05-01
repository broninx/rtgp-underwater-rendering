#version 410 core

layout(location = 0) out vec4 FragColor;

in vec2 Tex;
in vec3 Normal;
in vec4 viewPos;
in vec3 worldPos;

uniform sampler2D gTerrainTexture;
uniform vec3 gReversedLightDir;

uniform float densityFog;
uniform vec3 topColor;
uniform vec3 botColor;

uniform float gMinHeight;
uniform float gMaxHeight;

void main()
{
    // the fog factor is in base at the distance from the obj and the density of the fog
    float dist = length(viewPos.xyz);
    float fogFactor = 1.0 - exp(-densityFog * dist);

    //the fog color is in base at the high of the obj
    // vec3 dir = normalize(viewPos.xyz);
    // float t = smoothstep(-0.8, 0.8, dir.y);
    float t = smoothstep(gMinHeight, gMaxHeight, worldPos.y);

    vec3 fogColor = mix(botColor, topColor, t);

    vec3 TexColor = texture(gTerrainTexture, Tex).rgb;

    vec3 Normal_ = normalize(Normal);
    float Diffuse = dot(Normal_, gReversedLightDir);
    Diffuse = max(0.3f, Diffuse);

    vec3 finalColor = TexColor * Diffuse;
    finalColor = mix(finalColor, fogColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
    // FragColor = vec4(fogColor, 1.0);
}