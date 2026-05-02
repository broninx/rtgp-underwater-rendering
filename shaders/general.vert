#version 410 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 InTex;

// Instanced model matrix – four vec4 attributes
layout (location = 5) in vec4 instModelCol0;
layout (location = 6) in vec4 instModelCol1;
layout (location = 7) in vec4 instModelCol2;
layout (location = 8) in vec4 instModelCol3;
 
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// normals transformation matrix (= transpose of the inverse of the model-view matrix)

out vec2 Tex;
out vec3 vNormal;
out vec4 viewPosition;
out vec4 worldPos;

void main()
{

    mat4 modelMatrix = mat4(instModelCol0, instModelCol1, instModelCol2, instModelCol3);
    vec4 worldPosition = modelMatrix * vec4(Position, 1.0);
    vec4 viewPos = viewMatrix * worldPosition;
    gl_Position = projectionMatrix * viewPos;

    worldPos = worldPosition;
    viewPosition = viewPos;

    Tex = InTex;

    vNormal = normalize ( mat3(modelMatrix) * Normal);
}