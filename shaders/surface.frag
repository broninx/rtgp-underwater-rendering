#version 410 core
out vec4 FragColor;

in vec3 WorldPos;   
in vec4 viewPos;
in vec2 Tex; 

uniform sampler2D NormalMap;
uniform float uTime;
uniform vec3 topColor;
uniform vec3 botColor;
uniform vec3 camPos;
uniform float distortStr;
uniform float fresnelPow;
uniform float densityFog;
uniform float gMinHeight;
uniform float gMaxHeight;



void main()
{
    // direction from the fragment TO the camera
    vec3 viewDir = normalize(camPos - WorldPos);

    // scroll the normal map over time
    vec2 uv = WorldPos.xz * 0.002;
    vec2 scroll = uv + vec2(uTime * 0.04, uTime * 0.03);
    vec3 texNormal = texture(NormalMap, scroll).rgb * 2.0 - 1.0;

    vec3 N = normalize(vec3(texNormal.xy, 1.0)); 

    // reflection of view direction about the perturbed normal
    vec3 R = reflect(viewDir, N);

    // R.y determines how much sky we see (positive = looking up towards zenith)
    float t = smoothstep(-0.2, 0.5, R.y);
    vec3 reflectedSky = mix(botColor, topColor, t);

    // Fresnel: more water colour at glancing angles
    float fresnel = 1.0 - abs(dot(viewDir, N));
    fresnel = pow(fresnel, fresnelPow);

    vec3 waterColor = vec3(0.1, 0.3, 0.5); 

    vec3 finalColor = mix(reflectedSky, waterColor, fresnel * 0.85);

    // the fog factor is in base at the distance from the obj and the density of the fog
    float dist = length(viewPos.xyz);
    float fogFactor = 1.0 - exp(-densityFog * dist);

    //the fog color is in base at the high of the obj
    float depthFog = WorldPos.y + 1.0f * (WorldPos.y - camPos.y);
    t = smoothstep(gMinHeight, gMaxHeight, depthFog);

    vec3 fogColor = mix(botColor, topColor, t);

    finalColor = mix(finalColor, fogColor, fogFactor);
    FragColor = vec4(finalColor, 0.5 * (1.0 - fogFactor) );
}