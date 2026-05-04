#version 410 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 InTex;
layout (location = 2) in vec3 InNormal;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 Tex;
out vec3 vNormal;
out vec4 viewPosition;
out vec3 worldPos;

void main()
{
    vec4 worldPosition = vec4(Position, 1.0);
    vec4 viewPos = viewMatrix * worldPosition;
    gl_Position = projectionMatrix * viewPos;

    viewPosition = viewPos;
    worldPos = worldPosition.xyz;
    Tex = InTex;

    vNormal = normalize(mat3(viewMatrix) * InNormal);
}