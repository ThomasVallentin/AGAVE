#version 460 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uViewMatrix;
uniform mat4 uProjMatrix;


out VertexData
{
    vec3 position;
} outVertex;


void main() 
{
    vec4 position = uProjMatrix * uViewMatrix * vec4(aPosition, 1.0);
    outVertex.position = position.xyz;

    gl_Position = position;
}
