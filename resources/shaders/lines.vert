#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in mat4 aMatrix;

uniform mat4 uViewMatrix;
uniform mat4 uProjMatrix;


void main() 
{
    gl_Position = uProjMatrix * uViewMatrix * aMatrix * vec4(aPosition, 1.0);
}
