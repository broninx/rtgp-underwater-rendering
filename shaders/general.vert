#version 410 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 InTex;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// normals transformation matrix (= transpose of the inverse of the model-view matrix)
uniform mat3 normalMatrix;

out vec2 Tex;
out vec3 vNormal;

void main()
{
    vec4 mvPosition = viewMatrix * modelMatrix * vec4(Position, 1.0);

    Tex = InTex;

    vNormal = normalize ( Normal * normalMatrix);

    gl_Position = projectionMatrix * mvPosition;
}