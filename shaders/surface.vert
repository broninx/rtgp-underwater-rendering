#version 410 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 InTex;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 Tex;
out vec3 WorldPos;
out vec4 viewPos;

void main()
{
    vec4 worldPos = modelMatrix * vec4(Position, 1.0);
    WorldPos = worldPos.xyz;
    viewPos = viewMatrix * worldPos;
    gl_Position = projectionMatrix * viewMatrix * worldPos;
    Tex = InTex;
}