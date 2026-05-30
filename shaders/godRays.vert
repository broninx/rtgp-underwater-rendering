#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

// Instanced model matrix – four vec4 attributes
layout (location = 5) in vec4 instModelCol0;
layout (location = 6) in vec4 instModelCol1;
layout (location = 7) in vec4 instModelCol2;
layout (location = 8) in vec4 instModelCol3;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 vUV;

void main()
{
    mat4 modelMatrix = mat4(instModelCol0, instModelCol1, instModelCol2, instModelCol3);
    mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
    gl_Position = MVP * vec4(aPos, 1.0);
    vUV = aTexCoords;
}