#version 410 core

out vec4 FragColor;

in vec2 Tex;
in vec3 vNormal;
in vec4 viewPosition;
in vec4 worldPos;

uniform sampler2D gTexture;
uniform vec3 gReversedLightDir;

uniform float densityFog;
uniform vec3 topColor;
uniform vec3 botColor;
uniform float gMinHeight;
uniform float gMaxHeight;
uniform vec3 camPos;
void main()
{   

    // the fog factor is in base at the distance from the obj and the density of the fog
    float dist = length(viewPosition.xyz);
    float fogFactor = 1.0 - exp(-densityFog * dist);

    //the fog color is in base at the high of the obj
    // vec3 dir = normalize(viewPosition.xyz);
    // float t = smoothstep(-0.2, 0.5, dir.y);
    float depthFog = worldPos.y + 1.0f * (worldPos.y - camPos.y);
    float t = smoothstep(gMinHeight, gMaxHeight, depthFog);

    vec3 fogColor = mix(botColor, topColor, t);

    //lighting
    vec3 TexColor = texture(gTexture, Tex).rgb;
    vec3 N = normalize(vNormal);
    float Diffuse = dot(N, gReversedLightDir);
    Diffuse = max(0.3f, Diffuse);

    vec3 finalColor = TexColor * Diffuse;
    finalColor = mix(finalColor, fogColor, fogFactor);

    FragColor = vec4(finalColor, 1.0f);
} 