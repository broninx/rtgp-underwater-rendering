#version 410 core

in vec2 vUV;
in vec4 viewPos;

// uniform float intensity;
uniform float dayPhase;
uniform float densityFog;
uniform float distFromWater;

out vec4 FragColor;

void main()
{
    float rayStr = smoothstep(0.6f, 1.0f, dayPhase);

    // The quad’s UV.y = 0 at the water surface (top), UV.y = 1 at the end (bottom)
    float fade = 1.0 - vUV.y;          
    float alpha = pow(fade, 2.0) * rayStr;
    vec3 rayColor = vec3(0.4f, 0.7f, 1.0f);

    // the fog factor is in base at the distance from the obj and the density of the fog
    float fogFactor = 1.0 - exp(-densityFog * distFromWater);

    FragColor = vec4( rayColor * alpha, alpha * (1.0 - fogFactor));
}