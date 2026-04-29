#version 410 core

layout (location = 0) in vec3 aPos;

out vec3 vWorldPos;


uniform mat4 gView;
uniform mat4 gProj;


void main()
{
    mat4 VP = gProj * gView;
    vec4 pos = VP * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
    vWorldPos = aPos;
}
