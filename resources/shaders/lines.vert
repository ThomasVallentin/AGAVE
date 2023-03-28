#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in mat4 aMatrix;
layout(location = 5) in vec4 aColor;

uniform mat4 uViewMatrix;
uniform mat4 uProjMatrix;

out vec4 vColor;

void main() 
{
    vColor = aColor;
    gl_Position = uProjMatrix * uViewMatrix * aMatrix * vec4(aPosition, 1.0);
}
