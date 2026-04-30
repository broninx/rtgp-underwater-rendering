#version 410 core

in vec3 vWorldPos;

uniform bool IsDay;

out vec4 FragColor;

void main()
{
    vec3 dir = normalize(vWorldPos);
    float t = smoothstep(-0.2, 0.5, dir.y);

    vec3 topColor;
    vec3 botColor;
    IsDay ? topColor = vec3(0.25, 0.65, 0.95) : topColor = vec3(0.02, 0.04, 0.12);
    IsDay ? botColor = vec3(0.01, 0.05, 0.15) : botColor = vec3(0.0, 0.005, 0.02);

    vec3 color = mix(botColor, topColor, t);

    FragColor = vec4(color, 1.0);

}
