#version 410 core

in vec3 vWorldPos;

uniform vec3 topColor;
uniform vec3 botColor;

out vec4 FragColor;


void main()
{

    vec3 dir = normalize(vWorldPos);
    float t = smoothstep(-0.2, 0.5, dir.y);

    

    vec3 color = mix(botColor, topColor, t);

    FragColor = vec4(color, 1.0);

}
